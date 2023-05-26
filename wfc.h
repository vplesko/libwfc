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

struct wfc__MatView {
    const uint32_t *m;
    int w, h;
};

int wfc__matRcToInd(int w, int r, int c) {
    return r * w + c;
}

void wfc__matIndToRc(int w, int ind, int *r, int *c) {
    *r = ind / w;
    *c = ind % w;
}

uint32_t wfc__matGet(struct wfc__MatView m, int r, int c) {
    return m.m[wfc__matRcToInd(m.w, r, c)];
}

uint32_t wfc__matGetWrap(struct wfc__MatView m, int r, int c) {
    r = wfc__indWrap(r, m.h);
    c = wfc__indWrap(c, m.w);
    return wfc__matGet(m, r, c);
}

struct wfc__Pattern {
    int t, l;
    int freq;
};

int wfc__patternsEq(int n, struct wfc__MatView m,
    struct wfc__Pattern patt1, struct wfc__Pattern patt2) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            uint32_t a = wfc__matGetWrap(m, patt1.t + i, patt1.l + j);
            uint32_t b = wfc__matGetWrap(m, patt2.t + i, patt2.l + j);
            if (a != b) return 0;
        }
    }
    return 1;
}

struct wfc__Pattern* wfc__gatherPatterns(int n, struct wfc__MatView m, int *cnt) {
    struct wfc__Pattern *patts = NULL;

    int pattCnt = 0;
    for (int px = 0; px < m.w * m.h; ++px) {
        struct wfc__Pattern patt = {0};
        wfc__matIndToRc(m.w, px, &patt.t, &patt.l);

        int seenBefore = 0;
        for (int px1 = 0; !seenBefore && px1 < px; ++px1) {
            struct wfc__Pattern patt1 = {0};
            wfc__matIndToRc(m.w, px1, &patt1.t, &patt1.l);

            if (wfc__patternsEq(n, m, patt, patt1)) seenBefore = 1;
        }

        if (!seenBefore) ++pattCnt;
    }

    patts = malloc(pattCnt * sizeof(*patts));
    pattCnt = 0;
    for (int px = 0; px < m.w * m.h; ++px) {
        struct wfc__Pattern patt = {0};
        wfc__matIndToRc(m.w, px, &patt.t, &patt.l);
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

    struct wfc__MatView srcMat = {src, srcW, srcH};

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
    for (int r = 0; r < srcH; ++r) {
        for (int c = 0; c < srcW; ++c) {
            struct wfc__Pattern patt = {r, c, 0};

            for (int i = 0; i < pattCnt; ++i) {
                if (wfc__patternsEq(n, srcMat, patt, patts[i])) {
                    int red = patts[i].freq * 255 / mostCommonPatt;
                    dst[wfc__matRcToInd(dstW, r, c)] = 0xff000000 + red;
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
    if (wfc_generate(n, srcW, srcH, srcU32, dstW, dstH, dstU32) != 0) {
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
