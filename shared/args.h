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

    void *_dst;
};

struct args_Descr args_argInt(const char *name, int *dst) {
    if (name != NULL) assert(strlen(name) > 0);

    return (struct args_Descr){
        ._name = name,
        ._type = args__TypeInt,
        ._dst = dst,
    };
}

struct args_Descr args_argBool(const char *name, int *dst) {
    if (name != NULL) assert(strlen(name) > 0);

    return (struct args_Descr){
        ._name = name,
        ._type = args__TypeBool,
        ._dst = dst,
    };
}

struct args_Descr args_argString(const char *name, const char **dst) {
    if (name != NULL) assert(strlen(name) > 0);

    return (struct args_Descr){
        ._name = name,
        ._type = args__TypeString,
        ._dst = dst,
    };
}

// @TODO bools with -- and no value (meaning true)
// @TODO optional with default vals
// @TODO verify all req params before opt
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

int args__checkAllFlagsKnown(
    int argc, char *argv[],
    size_t len, const struct args_Descr *descrs) {
    for (int a = 1; a < argc;) {
        int isFlag = argv[a][0] == '-';

        if (isFlag) {
            int known = 0;

            for (size_t i = 0; i < len; ++i) {
                if (descrs[i]._name != NULL &&
                    strcmp(argv[a] + 1, descrs[i]._name) == 0) {
                    known = 1;
                    break;
                }
            }

            if (!known) {
                fprintf(stderr, "Invalid arguments.\n");
                return -1;
            }
        }

        a += 1 + isFlag;
    }

    return 0;
}

int args__parseFlags(
    int argc, char *argv[],
    size_t len, const struct args_Descr *descrs) {
    for (size_t i = 0; i < len; ++i) {
        const struct args_Descr *descr = &descrs[i];
        if (descr->_name == NULL) continue;

        int found = 0;

        for (int a = 1; a < argc;) {
            int isFlag = argv[a][0] == '-';

            if (isFlag && strcmp(argv[a] + 1, descr->_name) == 0) {
                if (found) {
                    fprintf(stderr, "Invalid arguments.\n");
                    return -1;
                }
                if (a + 1 >= argc) {
                    fprintf(stderr, "Invalid arguments.\n");
                    return -1;
                }

                if (args__parseVal(argv[a + 1], descr) < 0) return -1;

                found = 1;
            }

            a += 1 + isFlag;
        }

        if (!found) {
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

        int isFlag = argv[a][0] == '-';

        if (!isFlag) {
            if (args__parseVal(argv[a], descr) < 0) return -1;
            ++i;
        }

        a += 1 + isFlag;
    }

    for (; i < len; ++i) {
        if (descrs[i]._name == NULL) {
            fprintf(stderr, "Invalid arguments.\n");
            return -1;
        }
    }

    while (a < argc) {
        int isFlag = argv[a][0] == '-';

        if (!isFlag) {
            fprintf(stderr, "Invalid arguments.\n");
            return -1;
        }

        a += 1 + isFlag;
    }

    return 0;
}

int args_parse(
    int argc, char *argv[],
    size_t len, const struct args_Descr *descrs) {
    if (args__checkAllFlagsKnown(argc, argv, len, descrs) < 0) return -1;

    if (args__parseFlags(argc, argv, len, descrs) < 0) return -1;
    if (args__parseParams(argc, argv, len, descrs) < 0) return -1;

    return 0;
}
