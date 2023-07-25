#include "args.h"

struct Args {
    const char *imagePath;
    int wfcN;
    int wfcRot;
    int dstW, dstH;
};

// @TODO add other wfc options as flags
// @TODO rand seed as arg
int parseArgs(int argc, char *argv[], struct Args *args) {
    args->wfcRot = 0;

    struct args_Descr flag[] = {
        args_argString(NULL, 1, &args->imagePath),
        args_argInt("n", 1, &args->wfcN),
        args_argBool("rot", 0, &args->wfcRot),
        args_argInt("w", 1, &args->dstW),
        args_argInt("h", 1, &args->dstH),
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
