#include "args.h"

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
