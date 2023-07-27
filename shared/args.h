#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum args__Type {
    args__TypeInt,
    args__TypeBool,
    args__TypeString,
};

struct args_Param {
    const char *_name;
    enum args__Type _type;
    int _required;

    void *_dst;
};

struct args_Param args_paramInt(const char *name, int required, int *dst) {
    if (name != NULL) assert(strlen(name) > 0);

    return (struct args_Param){
        ._name = name,
        ._type = args__TypeInt,
        ._required = required,
        ._dst = dst,
    };
}

struct args_Param args_paramBool(const char *name, int required, int *dst) {
    if (name != NULL) assert(strlen(name) > 0);

    return (struct args_Param){
        ._name = name,
        ._type = args__TypeBool,
        ._required = required,
        ._dst = dst,
    };
}

struct args_Param args_paramString(
    const char *name, int required, const char **dst) {
    if (name != NULL) assert(strlen(name) > 0);

    return (struct args_Param){
        ._name = name,
        ._type = args__TypeString,
        ._required = required,
        ._dst = dst,
    };
}

// @TODO assert params have unique names
// @TODO param descriptions
// @TODO helpful error msgs
// @TODO help text
// @TODO no flags named help
// @TODO add arg to args_parse to allow ignoring unknown args
// @TODO revise how bools are parsed

int args__parseVal(const char *str, const struct args_Param *param) {
    if (param->_type == args__TypeInt) {
        long l;
        char *end;
        l = strtol(str, &end, 0);
        if (*end != '\0') {
            fprintf(stderr, "Invalid arguments.\n");
            return -1;
        }

        int i = (int)l;
        if (i != l) {
            fprintf(stderr, "Invalid arguments.\n");
            return -1;
        }

        if (param->_dst != NULL) *(int*)param->_dst = i;
    } else if (param->_type == args__TypeBool) {
        int b;
        if (strcmp(str, "t") == 0 ||
            strcmp(str, "T") == 0 ||
            strcmp(str, "true") == 0 ||
            strcmp(str, "True") == 0 ||
            strcmp(str, "1") == 0) {
            b = 1;
        } else if (strcmp(str, "f") == 0 ||
            strcmp(str, "F") == 0 ||
            strcmp(str, "false") == 0 ||
            strcmp(str, "False") == 0 ||
            strcmp(str, "0") == 0) {
            b = 0;
        } else {
            fprintf(stderr, "Invalid arguments.\n");
            return -1;
        }

        if (param->_dst != NULL) *(int*)param->_dst = b;
    } else if (param->_type == args__TypeString) {
        if (param->_dst != NULL) {
            *(const char**)param->_dst = str;
        }
    }

    return 0;
}

int args__paramIsFlag(const struct args_Param *param) {
    return param->_name != NULL;
}

int args__paramIsPos(const struct args_Param *param) {
    return param->_name == NULL;
}

int args__argIsFlag(const char *arg) {
    return arg[0] == '-';
}

int args__argIsPos(const char *arg) {
    return arg[0] != '-';
}

const char* args__argFlagName(const char *arg) {
    return arg + 1 + (arg[1] == '-');
}

int args__needsVal(const char *arg) {
    return arg[0] == '-' && arg[1] != '-';
}

int args__checkAllFlagsKnown(
    int argc, char * const *argv,
    size_t len, const struct args_Param *params) {
    for (int a = 1; a < argc;) {
        if (args__argIsFlag(argv[a])) {
            int known = 0;
            for (size_t i = 0; i < len; ++i) {
                if (args__paramIsFlag(&params[i]) &&
                    strcmp(args__argFlagName(argv[a]), params[i]._name) == 0) {
                    known = 1;
                    break;
                }
            }

            if (!known) {
                fprintf(stderr, "Invalid arguments.\n");
                return -1;
            }
        }

        a += 1 + args__needsVal(argv[a]);
    }

    return 0;
}

void args__assertAllReqPosBeforeOpt(
    size_t len, const struct args_Param *params) {
    int foundOpt = 0;
    for (size_t i = 0; i < len; ++i) {
        if (args__paramIsPos(&params[i])) {
            if (foundOpt) assert(!params[i]._required);

            if (!params[i]._required) foundOpt = 1;
        }
    }
}

int args__parseFlags(
    int argc, char * const *argv,
    size_t len, const struct args_Param *params) {
    for (size_t i = 0; i < len; ++i) {
        const struct args_Param *param = &params[i];
        if (!args__paramIsFlag(param)) continue;

        int found = 0;

        for (int a = 1; a < argc;) {
            if (args__argIsFlag(argv[a]) &&
                strcmp(args__argFlagName(argv[a]), param->_name) == 0) {
                if (found) {
                    fprintf(stderr, "Invalid arguments.\n");
                    return -1;
                }

                if (args__needsVal(argv[a])) {
                    if (a + 1 >= argc) {
                        fprintf(stderr, "Invalid arguments.\n");
                        return -1;
                    }

                    if (args__parseVal(argv[a + 1], param) < 0) return -1;
                } else {
                    if (param->_type != args__TypeBool) {
                        fprintf(stderr, "Invalid arguments.\n");
                        return -1;
                    }

                    *(int*)param->_dst = 1;
                }

                found = 1;
            }

            a += 1 + args__needsVal(argv[a]);
        }

        if (param->_required && !found) {
            fprintf(stderr, "Invalid arguments.\n");
            return -1;
        }
    }

    return 0;
}

int args__parsePos(
    int argc, char * const *argv,
    size_t len, const struct args_Param *params) {
    size_t i = 0;
    int a = 1;
    while (i < len && a < argc) {
        const struct args_Param *param = &params[i];
        if (!args__paramIsPos(param)) {
            ++i;
            continue;
        }

        if (args__argIsPos(argv[a])) {
            if (args__parseVal(argv[a], param) < 0) return -1;
            ++i;
        }

        a += 1 + args__needsVal(argv[a]);
    }

    for (; i < len; ++i) {
        if (args__paramIsPos(&params[i]) && params[i]._required) {
            fprintf(stderr, "Invalid arguments.\n");
            return -1;
        }
    }

    while (a < argc) {
        if (args__argIsPos(argv[a])) {
            fprintf(stderr, "Invalid arguments.\n");
            return -1;
        }

        a += 1 + args__needsVal(argv[a]);
    }

    return 0;
}

int args_parse(
    int argc, char * const *argv,
    size_t len, const struct args_Param *params) {
    assert(argc > 0);
    assert(argv != NULL);
    for (int a = 0; a < argc; ++a) {
        assert(argv[a] != NULL);
        assert(strlen(argv[a]) > 0);
    }

    if (len > 0) assert(params != NULL);

    if (args__checkAllFlagsKnown(argc, argv, len, params) < 0) return -1;
    args__assertAllReqPosBeforeOpt(len, params);

    if (args__parseFlags(argc, argv, len, params) < 0) return -1;
    if (args__parsePos(argc, argv, len, params) < 0) return -1;

    return 0;
}
