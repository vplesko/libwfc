struct Args {
    const char *pathIn;
    const char *pathOut;
    unsigned seed;
    int n;
    int dstW, dstH;
    bool flipH, flipV;
    bool rot;
    bool edgeH, edgeV;
};

int parseArgs(int argc, char * const *argv, struct Args *args, bool outReq) {
    args->seed = (unsigned)time(NULL);
    bool flip;
    bool edge;

    unargs_Param paramOut;
    if (outReq) {
        paramOut = unargs_stringReq(
            "o",
            "Output image file path.",
            &args->pathOut
        );
    } else {
        paramOut = unargs_string(
            "o",
            "Output image file path.",
            NULL,
            &args->pathOut
        );
    }

    unargs_Param params[] = {
        unargs_stringReq(
            NULL,
            "Input image file path.",
            &args->pathIn
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
        paramOut,
        unargs_unsigned(
            "seed",
            "Seed value for the random number generator.",
            args->seed,
            &args->seed
        ),
        unargs_bool(
            "flip-h",
            "Enables horizontal flipping of patterns"
            " (think of y-axis as the mirror).",
            &args->flipH
        ),
        unargs_bool(
            "flip-v",
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
        unargs_bool(
            "edge-h",
            "Fixes left and right edges"
            " so that patterns may not wrap around them.",
            &args->edgeH
        ),
        unargs_bool(
            "edge-v",
            "Fixes top and bottom edges"
            " so that patterns may not wrap around them.",
            &args->edgeV
        ),
        unargs_bool(
            "edge",
            "Fixes all four edges"
            " so that patterns may not wrap around them.",
            &edge
        ),
    };

    int status = unargs_parse(
        argc, argv, sizeof(params) / sizeof(*params), params);
    if (status == unargs_err_args) {
        fprintf(stdout, "\n");
        unargs_help(argv[0], sizeof(params) / sizeof(*params), params);
        return -1;
    }
    assert(status == unargs_ok);

    if (flip) {
        args->flipH = args->flipV = true;
    }
    if (edge) {
        args->edgeH = args->edgeV = true;
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

    if (args.pathOut != NULL && getImageFormat(args.pathOut) == IMG_INVALID) {
        fprintf(stderr,
            "Invalid output file name. Supported output extensions are:"
            " BMP, PNG, TGA.");
        return -1;
    }

    return 0;
}

int argsToWfcOptions(struct Args args) {
    int options = 0;
    if (args.flipH) options |= wfc_optFlipH;
    if (args.flipV) options |= wfc_optFlipV;
    if (args.rot) options |= wfc_optRotate;
    if (args.edgeH) options |= wfc_optEdgeFixH;
    if (args.edgeV) options |= wfc_optEdgeFixV;

    return options;
}

void printPrelude(struct Args args, int srcW, int srcH, int pattCnt) {
    fprintf(stdout, "Seed:       %d\n", args.seed);
    fprintf(stdout, "Input size: %dx%d\n", srcW, srcH);
    fprintf(stdout, "Patterns:   %d\n", pattCnt);
}
