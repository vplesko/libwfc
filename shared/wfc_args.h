#define UNARGS_IMPLEMENTATION
#include "unargs.h"

struct Args {
    const char *inPath;
    int wfcN;
    int dstW, dstH;
    bool wfcRot;
};

// @TODO add other wfc options as flags
// @TODO rand seed as arg
int parseArgs(int argc, char * const *argv, struct Args *args) {
    struct unargs_Param params[] = {
        unargs_stringReq(
            NULL,
            "Input image path.",
            &args->inPath
        ),
        unargs_intReq(
            "n",
            "N parameter for WFC - patterns will be NxN pixels.",
            &args->wfcN
        ),
        unargs_intReq(
            "w",
            "Width of the generated image.",
            &args->dstW
        ),
        unargs_intReq(
            "h",
            "Height of the generated image.",
            &args->dstH
        ),
        unargs_bool(
            "rot",
            "Enables rotating of patterns.",
            &args->wfcRot
        ),
    };

    if (unargs_parse(
        argc, argv,
        sizeof(params) / sizeof(*params), params) < 0) {
        unargs_help(argv[0], sizeof(params) / sizeof(*params), params);
        return -1;
    }

    // @TODO error messages for each
    if (args->wfcN <= 0 ||
        args->dstW <= 0 || args->dstH <= 0 ||
        args->wfcN > args->dstW || args->wfcN > args->dstH) {
        fprintf(stderr, "Invalid arguments.\n");
        return -1;
    }

    return 0;
}
