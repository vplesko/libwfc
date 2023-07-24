#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum ArgType {
    argTypeInt,
    argTypeString,
};

struct ArgDescr {
    const char *name;
    enum ArgType type;

    void *dst;
};

struct ArgDescr argInt(const char *name, int *dst) {
    assert(name != NULL);
    assert(strlen(name) > 0);

    return (struct ArgDescr){
        .name = name,
        .type = argTypeInt,
        .dst = dst,
    };
}

struct ArgDescr argString(const char *name, const char **dst) {
    assert(name != NULL);
    assert(strlen(name) > 0);

    return (struct ArgDescr){
        .name = name,
        .type = argTypeString,
        .dst = dst,
    };
}

// @TODO optional with default vals
// @TODO params (no name)
// @TODO assert that calls ok
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

int parseArgs(
    int argc, char *argv[],
    size_t len, const struct ArgDescr *descrs) {
    for (size_t i = 0; i < len; ++i) {
        const struct ArgDescr *descr = &descrs[i];

        int found = 0;

        for (int a = 1; a < argc; a += 2) {
            if (strcmp(argv[a] + 1, descr->name) == 0) {
                if (a + 1 >= argc) {
                    fprintf(stderr, "Invalid arguments.\n");
                    return -1;
                }

                if (descr->type == argTypeInt) {
                    long l;
                    char *end;
                    l = strtol(argv[a + 1], &end, 0);
                    if (*end != '\0') {
                        fprintf(stderr, "Invalid arguments.\n");
                        return -1;
                    }

                    int i = (int)l;
                    if (i != l) {
                        fprintf(stderr, "Invalid arguments.\n");
                        return -1;
                    }

                    if (descr->dst != NULL) *(int*)descr->dst = i;
                } else if (descr->type == argTypeString) {
                    if (descr->dst != NULL) {
                        *(const char **)descr->dst = argv[a + 1];
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

struct WfcArgs {
    const char *imagePath;
    int wfcN;
    int dstW, dstH;
};

// @TODO rand seed as arg
int parseWfcArgs(int argc, char *argv[], struct WfcArgs *args) {
    struct ArgDescr flag[] = {
        // @TODO turn path into a param
        argString("path", &args->imagePath),
        argInt("n", &args->wfcN),
        argInt("w", &args->dstW),
        argInt("h", &args->dstH),
    };
    if (parseArgs(argc, argv, sizeof(flag) / sizeof(*flag), flag) < 0) {
        return -1;
    }

    if (args->wfcN <= 0 ||
        args->dstW <= 0 || args->dstH <= 0 ||
        args->wfcN > args->dstW || args->wfcN > args->dstH) {
        fprintf(stderr, "Invalid arguments.\n");
        return -1;
    }

    return 0;
}
