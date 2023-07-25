#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum args__Type {
    args__TypeInt,
    args__TypeBool,
    args__TypeString,
};

struct args_Descr {
    const char *_name;
    enum args__Type _type;
    int _required;

    void *_dst;
};

struct args_Descr args_argInt(const char *name, int required, int *dst) {
    if (name != NULL) assert(strlen(name) > 0);

    return (struct args_Descr){
        ._name = name,
        ._type = args__TypeInt,
        ._required = required,
        ._dst = dst,
    };
}

struct args_Descr args_argBool(const char *name, int required, int *dst) {
    if (name != NULL) assert(strlen(name) > 0);

    return (struct args_Descr){
        ._name = name,
        ._type = args__TypeBool,
        ._required = required,
        ._dst = dst,
    };
}

struct args_Descr args_argString(
    const char *name, int required, const char **dst) {
    if (name != NULL) assert(strlen(name) > 0);

    return (struct args_Descr){
        ._name = name,
        ._type = args__TypeString,
        ._required = required,
        ._dst = dst,
    };
}

// @TODO helpful error msgs
// @TODO help text
// @TODO no flags named help

int args__parseVal(const char *str, const struct args_Descr *descr) {
    if (descr->_type == args__TypeInt) {
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

        if (descr->_dst != NULL) *(int*)descr->_dst = i;
    } else if (descr->_type == args__TypeBool) {
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

        if (descr->_dst != NULL) *(int*)descr->_dst = b;
    } else if (descr->_type == args__TypeString) {
        if (descr->_dst != NULL) {
            *(const char**)descr->_dst = str;
        }
    }

    return 0;
}

int args__isFlag(const char *arg) {
    return arg[0] == '-';
}

const char* args__flagName(const char *arg) {
    return arg + 1 + (arg[1] == '-');
}

int args__needsVal(const char *arg) {
    return arg[0] == '-' && arg[1] != '-';
}

int args__checkAllFlagsKnown(
    int argc, char *argv[],
    size_t len, const struct args_Descr *descrs) {
    for (int a = 1; a < argc;) {
        if (args__isFlag(argv[a])) {
            int known = 0;
            for (size_t i = 0; i < len; ++i) {
                if (descrs[i]._name != NULL &&
                    strcmp(args__flagName(argv[a]), descrs[i]._name) == 0) {
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

void args__assertAllReqParamsBeforeOpt(
    size_t len, const struct args_Descr *descrs) {
    int foundOpt = 0;
    for (size_t i = 0; i < len; ++i) {
        if (descrs[i]._name == NULL) {
            if (foundOpt) assert(!descrs[i]._required);

            if (!descrs[i]._required) foundOpt = 1;
        }
    }
}

int args__parseFlags(
    int argc, char *argv[],
    size_t len, const struct args_Descr *descrs) {
    for (size_t i = 0; i < len; ++i) {
        const struct args_Descr *descr = &descrs[i];
        if (descr->_name == NULL) continue;

        int found = 0;

        for (int a = 1; a < argc;) {
            if (args__isFlag(argv[a]) &&
                strcmp(args__flagName(argv[a]), descr->_name) == 0) {
                if (found) {
                    fprintf(stderr, "Invalid arguments.\n");
                    return -1;
                }

                if (args__needsVal(argv[a])) {
                    if (a + 1 >= argc) {
                        fprintf(stderr, "Invalid arguments.\n");
                        return -1;
                    }

                    if (args__parseVal(argv[a + 1], descr) < 0) return -1;
                } else {
                    if (descr->_type != args__TypeBool) {
                        fprintf(stderr, "Invalid arguments.\n");
                        return -1;
                    }

                    *(int*)descr->_dst = 1;
                }

                found = 1;
            }

            a += 1 + args__needsVal(argv[a]);
        }

        if (descr->_required && !found) {
            fprintf(stderr, "Invalid arguments.\n");
            return -1;
        }
    }

    return 0;
}

int args__parseParams(
    int argc, char *argv[],
    size_t len, const struct args_Descr *descrs) {
    size_t i = 0;
    int a = 1;
    while (i < len && a < argc) {
        const struct args_Descr *descr = &descrs[i];
        if (descr->_name != NULL) {
            ++i;
            continue;
        }

        if (!args__isFlag(argv[a])) {
            if (args__parseVal(argv[a], descr) < 0) return -1;
            ++i;
        }

        a += 1 + args__needsVal(argv[a]);
    }

    for (; i < len; ++i) {
        if (descrs[i]._name == NULL && descrs[i]._required) {
            fprintf(stderr, "Invalid arguments.\n");
            return -1;
        }
    }

    while (a < argc) {
        if (!args__isFlag(argv[a])) {
            fprintf(stderr, "Invalid arguments.\n");
            return -1;
        }

        a += 1 + args__needsVal(argv[a]);
    }

    return 0;
}

int args_parse(
    int argc, char *argv[],
    size_t len, const struct args_Descr *descrs) {
    assert(argc > 0);
    assert(argv != NULL);
    for (int a = 0; a < argc; ++a) {
        assert(argv[a] != NULL);
        assert(strlen(argv[a]) > 0);
    }

    if (len > 0) assert(descrs != NULL);

    if (args__checkAllFlagsKnown(argc, argv, len, descrs) < 0) return -1;
    args__assertAllReqParamsBeforeOpt(len, descrs);

    if (args__parseFlags(argc, argv, len, descrs) < 0) return -1;
    if (args__parseParams(argc, argv, len, descrs) < 0) return -1;

    return 0;
}
