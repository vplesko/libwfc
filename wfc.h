#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// @TODO implement
// @TODO allow users to supply their own assert
// @TODO allow users to supply their own malloc et al.
// @TODO does this API suffer from issues related to strict aliasing?

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
    int yy = ind / w;
    ind -= yy * w;
    int xx = ind;

    *x = xx;
    *y = yy;
}

#define WFC__MAT2DGET(mat, x, y) \
    (mat.m[wfc__mat2dXyToInd(mat.w, x, y)])

#define WFC__MAT2DGETWRAP(mat, x, y) \
    (mat.m[wfc__mat2dXyToInd( \
        mat.w, \
        wfc__indWrap(x, mat.w), \
        wfc__indWrap(y, mat.h))\
    ])

#define WFC__DEF_MAT3D(type, abbrv) \
    struct wfc__Mat3d_##abbrv { \
        type *m; \
        int w, h, d; \
    }

int wfc__mat3dXyzToInd(int w, int h, int x, int y, int z) {
    return z * w * h + y * w + x;
}

void wfc__mat3dIndToXyz(int w, int h, int ind, int *x, int *y, int *z) {
    int zz = ind / (w * h);
    ind -= zz * (w * h);
    int yy = ind / w;
    ind -= yy;
    int xx = ind;

    *x = xx;
    *y = yy;
    *z = zz;
}

#define WFC__MAT3DGET(mat, x, y, z) \
    (mat.m[wfc__mat3dXyzToInd(mat.w, mat.h, x, y, z)])

struct wfc__Pattern {
    int l, t;
    int freq;
};

WFC__DEF_MAT2D(const uint32_t, cu32);
// @TODO allow users to use double instead
WFC__DEF_MAT2D(float, f);
WFC__DEF_MAT3D(uint8_t, u8);

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

void wfc__calcEntropies(
    struct wfc__Pattern *patts,
    const struct wfc__Mat3d_u8 wave,
    struct wfc__Mat2d_f entropies) {
    for (int x = 0; x < entropies.w; ++x) {
        for (int y = 0; y < entropies.h; ++y) {
            int totalFreq = 0;
            for (int z = 0; z < wave.d; ++z) {
                if (WFC__MAT3DGET(wave, x, y, z)) {
                    totalFreq += patts[z].freq;
                }
            }

            float entropy = 0.0f;
            for (int z = 0; z < wave.d; ++z) {
                if (WFC__MAT3DGET(wave, x, y, z)) {
                    float prob = 1.0f * patts[z].freq / totalFreq;
                    entropy -= prob * log2f(prob);
                }
            }

            WFC__MAT2DGET(entropies, x, y) = entropy;
        }
    }
}

int wfc_generate(
    int n,
    int srcW, int srcH, const uint32_t *src,
    int dstW, int dstH, uint32_t *dst) {
    struct wfc__Pattern *patts = NULL;
    struct wfc__Mat3d_u8 wave = {0};
    struct wfc__Mat2d_f entropies = {0};

    struct wfc__Mat2d_cu32 srcMat = {src, srcW, srcH};

    int pattCnt;
    patts = wfc__gatherPatterns(n, srcMat, &pattCnt);

    wave.w = dstW;
    wave.h = dstH;
    wave.d = pattCnt;
    wave.m = malloc(wave.w * wave.h * wave.d * sizeof(*wave.m));
    for (int i = 0; i < wave.w * wave.h * wave.d; ++i) wave.m[i] = 1;

    entropies.w = dstW;
    entropies.h = dstH;
    entropies.m = malloc(entropies.w * entropies.h * sizeof(*entropies.m));
    wfc__calcEntropies(patts, wave, entropies);

    // dummy impl
    for (int px = 0; px < dstW * dstH; ++px) {
        dst[px] = 0xff7f7f7f;
    }
    int mostCommonPatt = 0;
    for (int i = 0; i < pattCnt; ++i) {
        if (patts[i].freq > mostCommonPatt) mostCommonPatt = patts[i].freq;
    }
    for (int x = 0; x < dstW; ++x) {
        for (int y = 0; y < dstH; ++y) {
            float entropy = WFC__MAT2DGET(entropies, x, y);

            const float xMid = 1;
            const float k = logf(2) / xMid;
            float fx = 1 - exp(-k * entropy);

            int blue = (int)(fx * 256.0f);
            dst[wfc__mat2dXyToInd(dstW, x, y)] = 0xff000000 + (blue << 16);
        }
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
    free(entropies.m);
    free(wave.m);
    free(patts);

    return 0;
}

int wfc_generatePixels(
    int n,
    int bytesPerPixel, uint32_t mask,
    int srcW, int srcH, int srcPitch, const unsigned char *src,
    int dstW, int dstH, int dstPitch, unsigned char *dst) {
    assert(bytesPerPixel >= 1 && bytesPerPixel <= 4);

    if (bytesPerPixel == 4 && mask == (uint32_t)-1 &&
        srcPitch == srcW * bytesPerPixel && dstPitch == dstW * bytesPerPixel) {
        return wfc_generate(
            n,
            srcW, srcH, (uint32_t*)src,
            dstW, dstH, (uint32_t*)dst);
    }

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
