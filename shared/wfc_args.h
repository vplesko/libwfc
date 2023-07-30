struct Args {
    const char *inPath;
    int n;
    int dstW, dstH;
    bool flipH, flipV;
    bool rot;
};

// @TODO add other wfc options as flags
// @TODO rand seed as arg
int parseArgs(int argc, char * const *argv, struct Args *args) {
    bool flip;

    struct unargs_Param params[] = {
        unargs_stringReq(
            NULL,
            "Input image path.",
            &args->inPath
        ),
        unargs_intReq(
            "n",
            "N parameter for WFC - patterns will be NxN pixels.",
            &args->n
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
            "fliph",
            "Enables horizontal flipping of patterns"
            " (think of y-axis as the mirror).",
            &args->flipH
        ),
        unargs_bool(
            "flipv",
            "Enables vertical flipping of patterns"
            " (think of x-axis as the mirror).",
            &args->flipV
        ),
        unargs_bool(
            "flip",
            "Enables flipping of patterns both horizontally and vertically.",
            &flip
        ),
        unargs_bool(
            "rot",
            "Enables rotating of patterns.",
            &args->rot
        ),
    };

    if (unargs_parse(
        argc, argv,
        sizeof(params) / sizeof(*params), params) < 0) {
        unargs_help(argv[0], sizeof(params) / sizeof(*params), params);
        return -1;
    }

    if (flip) {
        args->flipH = args->flipV = true;
    }

    return 0;
}

int verifyArgs(struct Args args, int srcW, int srcH) {
    if (args.n <= 0) {
        fprintf(stderr, "N must be positive.\n");
        return -1;
    }

    if (args.dstW <= 0 || args.dstH <= 0) {
        fprintf(stderr, "Output image dimensions must be positive.\n");
        return -1;
    }

    if (args.n > srcW || args.n > srcH ||
        args.n > args.dstW || args.n > args.dstH) {
        fprintf(stderr,
            "N must not be greater than"
            " any input or output image dimensions.\n");
        return -1;
    }

    return 0;
}

int argsToWfcOptions(struct Args args) {
    int options = 0;
    if (args.flipH) options |= wfc_optFlipH;
    if (args.flipV) options |= wfc_optFlipV;
    if (args.rot) options |= wfc_optRotate;

    return options;
}
