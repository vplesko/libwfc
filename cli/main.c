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
            args.dstW, args.dstH,
            &wfc) != 0) {
        fprintf(stderr, "WFC init failed.\n");
        ret = 1;
        goto cleanup;
    }

    while (1) {
        int status = wfcStep(&wfc);
        if (status == wfc_failed) {
            if (wfcBacktrack(&wfc) != 0) {
                fprintf(stderr, "WFC step failed.\n");
                ret = 1;
                goto cleanup;
            } else {
                fprintf(stdout, "WFC is backtracking.\n");
            }
        } else if (status == wfc_completed) {
            break;
        } else {
            assert(status == 0);
        }
    }

    wfcBlit(wfc, srcPixels, dstPixels);

    enum ImageFormat fmt = getImageFormat(args.pathOut);
    if (fmt == IMG_BMP) {
        if (stbi_write_bmp(args.pathOut,
                args.dstW, args.dstH, bytesPerPixel, dstPixels) == 0) {
            fprintf(stderr, "Error writing to file %s\n", args.pathOut);
            ret = 1;
            goto cleanup;
        }
    } else if (fmt == IMG_PNG) {
        if (stbi_write_png(args.pathOut,
                args.dstW, args.dstH, bytesPerPixel, dstPixels,
                args.dstW * bytesPerPixel) == 0) {
            fprintf(stderr, "Error writing to file %s\n", args.pathOut);
            ret = 1;
            goto cleanup;
        }
    } else if (fmt == IMG_TGA) {
        if (stbi_write_tga(args.pathOut,
                args.dstW, args.dstH, bytesPerPixel, dstPixels) == 0) {
            fprintf(stderr, "Error writing to file %s\n", args.pathOut);
            ret = 1;
            goto cleanup;
        }
    } else {
        assert(false);
    }

cleanup:
    wfcFree(wfc);
    free(dstPixels);
    stbi_image_free(srcPixels);

    return ret;
}
