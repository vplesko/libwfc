#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// @TODO implement

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
    int ret = 0;

    uint32_t *srcI = NULL;
    uint32_t *dstI = NULL;

    // @TODO if bytesPerPixel <= sizeof(int), each pixel can be simply cast to int
    srcI = malloc(srcW * srcH * sizeof(*srcI));
    for (int px = 0; px < srcW * srcH; ++px) {
        srcI[px] = px;
        for (int pxPrev = 0; pxPrev < px; ++pxPrev) {
            if (memcmp(
                    src + px * bytesPerPixel,
                    src + pxPrev * bytesPerPixel,
                    bytesPerPixel) == 0) {
                srcI[px] = pxPrev;
                break;
            }
        }
    }

    dstI = malloc(dstW * dstH * sizeof(*dstI));
    if (wfc_generate(srcW, srcH, srcI, dstW, dstH, dstI) != 0) {
        ret = 1;
        goto cleanup;
    }

    for (int px = 0; px < dstW * dstH; ++px) {
        memcpy(
            dst + px * bytesPerPixel,
            src + dstI[px] * bytesPerPixel,
            bytesPerPixel);
    }

cleanup:
    free(dstI);
    free(srcI);

    return ret;
}
