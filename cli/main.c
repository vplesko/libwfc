#include <stdio.h>
#include <time.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "cmd_args.h"
#include "wfc_wrap.h"

int main(int argc, char *argv[]) {
    int ret = 0;

    const int bytesPerPixel = 4;

    unsigned char *srcPixels = NULL;
    unsigned char *dstPixels = NULL;
    struct WfcWrapper wfc = {0};

    struct WfcArgs args;
    if (parseWfcArgs(argc, argv, &args) != 0) {
        ret = 1;
        goto cleanup;
    }

    int srcW, srcH;
    srcPixels = stbi_load(args.imagePath, &srcW, &srcH, NULL, bytesPerPixel);
    if (srcPixels == NULL) {
        fprintf(stderr, "%s\n", stbi_failure_reason());
        ret = 1;
        goto cleanup;
    }

    srand((unsigned)time(NULL));

    dstPixels = malloc(args.dstW * args.dstH * bytesPerPixel);

    if (wfcInit(
            args.wfcN, 0, bytesPerPixel,
            srcW, srcH, srcPixels,
            args.dstW, args.dstH,
            &wfc) != 0) {
        fprintf(stderr, "WFC init failed.\n");
        ret = 1;
        goto cleanup;
    }

    while (1) {
        int status = wfcStep(&wfc);
        if (status < 0) {
            if (wfcBacktrack(&wfc) != 0) {
                fprintf(stderr, "WFC step failed.\n");
                ret = 1;
                goto cleanup;
            } else {
                fprintf(stdout, "WFC is backtracking.\n");
            }
        } else if (status > 0) {
            break;
        }
    }

    wfcBlit(wfc, srcPixels, dstPixels);

    // dummy printout to make sure the compiler doesn't optimize anything away
    uint32_t dummy = 0;
    for (int i = 0; i < args.dstW * args.dstH * bytesPerPixel; ++i) {
        dummy += dstPixels[i];
    }
    printf("%u\n", dummy);

cleanup:
    wfcFree(wfc);
    free(dstPixels);
    stbi_image_free(srcPixels);

    return ret;
}
