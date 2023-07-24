#include "args.h"

struct Args {
    const char *imagePath;
    int wfcN;
    int dstW, dstH;
};

// @TODO rand seed as arg
int parseArgs(int argc, char *argv[], struct Args *args) {
    struct args_Descr flag[] = {
        args_argString(NULL, &args->imagePath),
        args_argInt("n", &args->wfcN),
        args_argInt("w", &args->dstW),
        args_argInt("h", &args->dstH),
    };
    if (args_parse(argc, argv, sizeof(flag) / sizeof(*flag), flag) < 0) {
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
