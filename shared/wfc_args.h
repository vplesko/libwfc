#include "args.h"

struct Args {
    const char *imagePath;
    int wfcN;
    int wfcRot;
    int dstW, dstH;
};

// @TODO add other wfc options as flags
// @TODO rand seed as arg
int parseArgs(int argc, char * const *argv, struct Args *args) {
    args->wfcRot = 0;

    struct args_Param flag[] = {
        args_paramString(NULL, 1, &args->imagePath),
        args_paramInt("n", 1, &args->wfcN),
        args_paramBool("rot", 0, &args->wfcRot),
        args_paramInt("w", 1, &args->dstW),
        args_paramInt("h", 1, &args->dstH),
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
