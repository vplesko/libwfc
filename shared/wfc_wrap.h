#include "wfc.h"

struct WfcWrapper {
    struct wfc_State *wfc;
    int bytesPerPixel;
};

int wfcGenerate(
    int n, int options, int bytesPerPixel,
    int srcW, int srcH, const unsigned char *src,
    int dstW, int dstH, unsigned char *dst) {
    return wfc_generate(n, options, bytesPerPixel,
        srcW, srcH, src, dstW, dstH, dst);
}

int wfcInit(
    int n, int options, int bytesPerPixel,
    int srcW, int srcH, const unsigned char *src,
    int dstW, int dstH,
    struct WfcWrapper *wfc) {
    wfc->wfc = wfc_init(n, options, bytesPerPixel, srcW, srcH, src, dstW, dstH);
    if (wfc->wfc == NULL) return -1;

    wfc->bytesPerPixel = bytesPerPixel;

    return 0;
}

int wfcStep(struct WfcWrapper wfc) {
    return wfc_step(wfc.wfc);
}

void wfcBlit(
    const struct WfcWrapper wfc,
    const unsigned char *src, unsigned char *dst) {
    wfc_blit(wfc.wfc, src, dst);
}

void wfcBlitAveraged(
    const struct WfcWrapper wfc,
    const unsigned char *src,
    int dstW, int dstH, unsigned char *dst) {
    int bytesPerPixel = wfc.bytesPerPixel;
    int pattCnt = wfc_patternCount(wfc.wfc);

    for (int j = 0; j < dstH; ++j) {
        for (int i = 0; i < dstW; ++i) {
            for (int b = 0; b < bytesPerPixel; ++b) {
                int sum = 0, cnt = 0;
                for (int p = 0; p < pattCnt; ++p) {
                    if (wfc_patternAvailable(wfc.wfc, p, i, j)) {
                        const unsigned char* px =
                            wfc_pixelToBlit(wfc.wfc, p, i, j, src);

                        sum += (int)px[b];
                        ++cnt;
                    }
                }

                unsigned char avg = (unsigned char)(sum / cnt);
                dst[j * dstW * bytesPerPixel + i * bytesPerPixel + b] = avg;
            }
        }
    }
}

void wfcFree(struct WfcWrapper wfc) {
    if (wfc.wfc != NULL) wfc_free(wfc.wfc);
}
