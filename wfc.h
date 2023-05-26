#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// @TODO implement
// @TODO allow users to supply their own assert
// @TODO allow users to supply their own malloc et al.

int wfc_generate(
    int srcW, int srcH, const uint32_t *src,
    int dstW, int dstH, uint32_t *dst) {
    int ind = 0;
    for (int px = 0; px < dstW * dstH; ++px) {
        dst[px] = src[ind];

        ++ind;
        if (ind == srcW * srcH) ind = 0;
    }

    return 0;
}

int wfc_generatePixels(
    int bytesPerPixel,
    int srcW, int srcH, const unsigned char *src,
    int dstW, int dstH, unsigned char *dst) {
    assert(bytesPerPixel >= 1 && bytesPerPixel <= 4);

    int ret = 0;

    uint32_t *srcU32 = NULL;
    uint32_t *dstU32 = NULL;

    srcU32 = malloc(srcW * srcH * sizeof(*srcU32));
    for (int px = 0; px < srcW * srcH; ++px) {
        srcU32[px] = 0;
        // @TODO cover the big-endian case
        memcpy(srcU32 + px, src + px * bytesPerPixel, bytesPerPixel);
    }

    dstU32 = malloc(dstW * dstH * sizeof(*dstU32));
    if (wfc_generate(srcW, srcH, srcU32, dstW, dstH, dstU32) != 0) {
        ret = 1;
        goto cleanup;
    }

    for (int px = 0; px < dstW * dstH; ++px) {
        memcpy(dst + px * bytesPerPixel, dstU32 + px, bytesPerPixel);
    }

cleanup:
    free(dstU32);
    free(srcU32);

    return ret;
}
