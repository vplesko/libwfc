#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "stb_image.h"
#include "stb_image_write.h"

#define UNARGS_IMPLEMENTATION
#include "unargs.h"

#include "util.h"
#define WFC_IMPLEMENTATION
#include "wfc.h"
#include "wfc_args.h"
#include "wfc_wrap.h"

int main(int argc, char *argv[]) {
    int ret = 0;

    const int bytesPerPixel = 4;

    unsigned char *srcPixels = NULL;
    unsigned char *dstPixels = NULL;
    struct WfcWrapper wfc = {0};

    struct Args args;
    if (parseArgs(argc, argv, &args, true) != 0) {
        ret = 1;
        goto cleanup;
    }

    int srcW, srcH;
    srcPixels = stbi_load(args.pathIn, &srcW, &srcH, NULL, bytesPerPixel);
    if (srcPixels == NULL) {
        fprintf(stderr, "Error opening file %s: %s\n",
            args.pathIn, stbi_failure_reason());
        ret = 1;
        goto cleanup;
    }

    if (verifyArgs(args, srcW, srcH) < 0) {
        ret = 1;
        goto cleanup;
    }

    dstPixels = malloc(args.dstW * args.dstH * bytesPerPixel);

    srand(args.seed);
    int wfcOptions = argsToWfcOptions(args);

    if (wfcInit(
            args.n, wfcOptions, bytesPerPixel,
            srcW, srcH, srcPixels,
            args.dstW, args.dstH, NULL,
            NULL,
            &wfc) != 0) {
        fprintf(stderr, "WFC init failed.\n");
        ret = 1;
        goto cleanup;
    }

    printPrelude(args, srcW, srcH, wfcPatternCount(wfc));
    fprintf(stdout, "\n");

    int printCounter = 0;

    while (1) {
        int status = wfcStep(&wfc);
        if (status == wfc_failed) {
            if (wfcBacktrack(&wfc) != 0) {
                fprintf(stdout, "WFC failed.\n");
                ret = 1;
                goto cleanup;
            } else {
                fprintf(stdout, "WFC is backtracking.\n");
            }
        } else if (status == wfc_completed) {
            fprintf(stdout, "WFC completed.\n");
            break;
        } else {
            assert(status == 0);
        }

        if (++printCounter == 100) {
            fprintf(stdout,
                "%d/%d\r", wfcCollapsedCount(wfc), args.dstW * args.dstH);

            printCounter = 0;
        }
    }

    wfcBlit(wfc, srcPixels, dstPixels);

    if (writeOut(&args, bytesPerPixel, dstPixels) != 0) {
        ret = 1;
        goto cleanup;
    }

cleanup:
    wfcFree(wfc);
    free(dstPixels);
    stbi_image_free(srcPixels);

    return ret;
}
