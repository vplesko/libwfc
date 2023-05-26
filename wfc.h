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
    int bytesPerPixel, uint32_t mask,
    int srcW, int srcH, int srcPitch, const unsigned char *src,
    int dstW, int dstH, int dstPitch, unsigned char *dst) {
    assert(bytesPerPixel >= 1 && bytesPerPixel <= 4);

    int ret = 0;

    uint32_t *srcU32 = NULL;
    uint32_t *dstU32 = NULL;

    srcU32 = malloc(srcW * srcH * sizeof(*srcU32));
    for (int r = 0; r < srcH; ++r) {
        for (int c = 0; c < srcW; ++c) {
            int px = r * srcW + c;
            int srcInd = r * srcPitch + c * bytesPerPixel;

            srcU32[px] = 0;
            // @TODO cover the big-endian case
            memcpy(srcU32 + px, src + srcInd, bytesPerPixel);
            srcU32[px] &= mask;
        }
    }

    dstU32 = malloc(dstW * dstH * sizeof(*dstU32));
    if (wfc_generate(srcW, srcH, srcU32, dstW, dstH, dstU32) != 0) {
        ret = 1;
        goto cleanup;
    }

    for (int r = 0; r < dstH; ++r) {
        for (int c = 0; c < dstW; ++c) {
            int px = r * dstW + c;
            int dstInd = r * dstPitch + c * bytesPerPixel;

            memcpy(dst + dstInd, dstU32 + px, bytesPerPixel);
        }
    }

cleanup:
    free(dstU32);
    free(srcU32);

    return ret;
}
