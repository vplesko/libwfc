#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum args__Type {
    args__TypeInt,
    args__TypeString,
};

struct args_Descr {
    const char *_name;
    enum args__Type _type;

    void *_dst;
};

struct args_Descr args_argInt(const char *name, int *dst) {
    assert(name != NULL);
    assert(strlen(name) > 0);

    return (struct args_Descr){
        ._name = name,
        ._type = args__TypeInt,
        ._dst = dst,
    };
}

struct args_Descr args_argString(const char *name, const char **dst) {
    assert(name != NULL);
    assert(strlen(name) > 0);

    return (struct args_Descr){
        ._name = name,
        ._type = args__TypeString,
        ._dst = dst,
    };
}

// @TODO optional with default vals
// @TODO params (no name)
/*
    @TODO verify:
        required params given
        no duplicate flags
        no unknown flags
        no more params than known
        all req params before opt
        no flags named help
*/
// @TODO bools with -- and no value (meaning true)
// @TODO helpful error msgs
// @TODO help text

int args__parseInt(const char *str, int *dst) {
    long l;
    char *end;
    l = strtol(str, &end, 0);
    if (*end != '\0') return -1;

    int i = (int)l;
    if (i != l) return -1;

    if (dst != NULL) *dst = i;
    return 0;
}

int args_parse(
    int argc, char *argv[],
    size_t len, const struct args_Descr *descrs) {
    for (size_t i = 0; i < len; ++i) {
        const struct args_Descr *descr = &descrs[i];

        int found = 0;

        for (int a = 1; a < argc; a += 2) {
            if (strcmp(argv[a] + 1, descr->_name) == 0) {
                if (a + 1 >= argc) {
                    fprintf(stderr, "Invalid arguments.\n");
                    return -1;
                }

                if (descr->_type == args__TypeInt) {
                    int i;
                    if (args__parseInt(argv[a + 1], &i) < 0) {
                        fprintf(stderr, "Invalid arguments.\n");
                        return -1;
                    }

                    if (descr->_dst != NULL) *(int*)descr->_dst = i;
                } else if (descr->_type == args__TypeString) {
                    if (descr->_dst != NULL) {
                        *(const char**)descr->_dst = argv[a + 1];
                    }
                }

                found = 1;
                break;
            }
        }

        if (!found) {
            fprintf(stderr, "Invalid arguments.\n");
            return -1;
        }
    }

    return 0;
}
