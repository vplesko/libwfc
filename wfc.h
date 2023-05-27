#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// @TODO implement
// @TODO allow users to supply their own assert
// @TODO allow users to supply their own malloc et al.

int wfc__indWrap(int ind, int sz) {
    if (ind >= 0) return ind % sz;
    return sz + ind % sz;
}

#define WFC__DEF_MAT2D(type, abbrv) \
    struct wfc__Mat2d_##abbrv { \
        type *m; \
        int w, h; \
    }

int wfc__mat2dXyToInd(int w, int x, int y) {
    return y * w + x;
}

void wfc__mat2dIndToXy(int w, int ind, int *x, int *y) {
    *y = ind / w;
    *x = ind % w;
}

#define WFC__MAT2DGET(m, x, y) (m.m[wfc__mat2dXyToInd(m.w, x, y)])

#define WFC__MAT2DGETWRAP(m, x, y) \
    (m.m[wfc__mat2dXyToInd(m.w, wfc__indWrap(x, m.w), wfc__indWrap(y, m.h))])

struct wfc__Pattern {
    int l, t;
    int freq;
};

WFC__DEF_MAT2D(const uint32_t, cu32);

int wfc__patternsEq(int n, struct wfc__Mat2d_cu32 m,
    struct wfc__Pattern patt1, struct wfc__Pattern patt2) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            uint32_t a = WFC__MAT2DGETWRAP(m, patt1.l + i, patt1.t + j);
            uint32_t b = WFC__MAT2DGETWRAP(m, patt2.l + i, patt2.t + j);
            if (a != b) return 0;
        }
    }
    return 1;
}

struct wfc__Pattern* wfc__gatherPatterns(int n, struct wfc__Mat2d_cu32 m,
    int *cnt) {
    struct wfc__Pattern *patts = NULL;

    int pattCnt = 0;
    for (int px = 0; px < m.w * m.h; ++px) {
        struct wfc__Pattern patt = {0};
        wfc__mat2dIndToXy(m.w, px, &patt.l, &patt.t);

        int seenBefore = 0;
        for (int px1 = 0; !seenBefore && px1 < px; ++px1) {
            struct wfc__Pattern patt1 = {0};
            wfc__mat2dIndToXy(m.w, px1, &patt1.l, &patt1.t);

            if (wfc__patternsEq(n, m, patt, patt1)) seenBefore = 1;
        }

        if (!seenBefore) ++pattCnt;
    }

    patts = malloc(pattCnt * sizeof(*patts));
    pattCnt = 0;
    for (int px = 0; px < m.w * m.h; ++px) {
        struct wfc__Pattern patt = {0};
        wfc__mat2dIndToXy(m.w, px, &patt.l, &patt.t);
        patt.freq = 1;

        int seenBefore = 0;
        for (int i = 0; !seenBefore && i < pattCnt; ++i) {
            if (wfc__patternsEq(n, m, patt, patts[i])) {
                ++patts[i].freq;
                seenBefore = 1;
            }
        }

        if (!seenBefore) patts[pattCnt++] = patt;
    }

    *cnt = pattCnt;
    return patts;
}

int wfc_generate(
    int n,
    int srcW, int srcH, const uint32_t *src,
    int dstW, int dstH, uint32_t *dst) {
    struct wfc__Pattern *patts = NULL;

    struct wfc__Mat2d_cu32 srcMat = {src, srcW, srcH};

    int pattCnt;
    patts = wfc__gatherPatterns(n, srcMat, &pattCnt);

    // dummy impl
    for (int px = 0; px < dstW * dstH; ++px) {
        dst[px] = 0xff7f7f7f;
    }
    int mostCommonPatt = 0;
    for (int i = 0; i < pattCnt; ++i) {
        if (patts[i].freq > mostCommonPatt) mostCommonPatt = patts[i].freq;
    }
    for (int x = 0; x < srcW; ++x) {
        for (int y = 0; y < srcH; ++y) {
            struct wfc__Pattern patt = {x, y, 0};

            for (int i = 0; i < pattCnt; ++i) {
                if (wfc__patternsEq(n, srcMat, patt, patts[i])) {
                    int red = patts[i].freq * 255 / mostCommonPatt;
                    dst[wfc__mat2dXyToInd(dstW, x, y)] = 0xff000000 + red;
                }
            }
        }
    }

//cleanup:
    free(patts);

    return 0;
}

int wfc_generatePixels(
    int n,
    int bytesPerPixel, uint32_t mask,
    int srcW, int srcH, int srcPitch, const unsigned char *src,
    int dstW, int dstH, int dstPitch, unsigned char *dst) {
    assert(bytesPerPixel >= 1 && bytesPerPixel <= 4);

    int ret = 0;

    uint32_t *srcU32 = NULL;
    uint32_t *dstU32 = NULL;

    srcU32 = malloc(srcW * srcH * sizeof(*srcU32));
    for (int x = 0; x < srcW; ++x) {
        for (int y = 0; y < srcH; ++y) {
            int px = y * srcW + x;
            int srcInd = y * srcPitch + x * bytesPerPixel;

            srcU32[px] = 0;
            // @TODO cover the big-endian case
            memcpy(srcU32 + px, src + srcInd, bytesPerPixel);
            srcU32[px] &= mask;
        }
    }

    dstU32 = malloc(dstW * dstH * sizeof(*dstU32));
    if (wfc_generate(n, srcW, srcH, srcU32, dstW, dstH, dstU32) != 0) {
        ret = 1;
        goto cleanup;
    }

    for (int x = 0; x < dstW; ++x) {
        for (int y = 0; y < dstH; ++y) {
            int px = y * dstW + x;
            int dstInd = y * dstPitch + x * bytesPerPixel;

            memcpy(dst + dstInd, dstU32 + px, bytesPerPixel);
        }
    }

cleanup:
    free(dstU32);
    free(srcU32);

    return ret;
}
