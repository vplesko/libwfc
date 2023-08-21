/*
MIT License

Copyright (c) 2023 Vladimir Pleskonjic

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef INCLUDE_WFC_H
#define INCLUDE_WFC_H

#include <stdbool.h>

// @TODO implement 3D WFC, with GUI support

#ifdef __cplusplus
extern "C" {
#endif

enum {
    wfc_completed = 1,
    wfc_failed = -1,
    wfc_callerError = -2
};

enum {
    wfc_optFlipH = 1 << 1,
    wfc_optFlipV = 1 << 0,

    wfc_optFlipC0 = wfc_optFlipV,
    wfc_optFlipC1 = wfc_optFlipH,

    wfc_optRotate = 1 << 2,

    wfc_optEdgeFixH = 1 << 4,
    wfc_optEdgeFixV = 1 << 3,

    wfc_optEdgeFixC0 = wfc_optEdgeFixV,
    wfc_optEdgeFixC1 = wfc_optEdgeFixH
};

typedef struct wfc_State wfc_State;

int wfc_generate(
    int n, int options, int bytesPerPixel,
    int srcW, int srcH, const unsigned char *src,
    int dstW, int dstH, unsigned char *dst);

int wfc_generateEx(
    int n, int options, int bytesPerPixel,
    int srcW, int srcH, const unsigned char *src,
    int dstW, int dstH, unsigned char *dst,
    void *ctx,
    bool *keep);

wfc_State* wfc_init(
    int n, int options, int bytesPerPixel,
    int srcW, int srcH, const unsigned char *src,
    int dstW, int dstH);

wfc_State* wfc_initEx(
    int n, int options, int bytesPerPixel,
    int srcW, int srcH, const unsigned char *src,
    int dstW, int dstH, const unsigned char *dst,
    void *ctx,
    bool *keep);

int wfc_status(const wfc_State *state);

int wfc_step(wfc_State *state);

int wfc_blit(
    const wfc_State *state,
    const unsigned char *src, unsigned char *dst);

wfc_State* wfc_clone(const wfc_State *state);

void wfc_free(wfc_State *state);

int wfc_patternCount(const wfc_State *state);

int wfc_patternAvailable(const wfc_State *state, int patt, int x, int y);

const unsigned char* wfc_pixelToBlit(const wfc_State *state,
    int patt, int x, int y, const unsigned char *src);

#ifdef __cplusplus
}
#endif

#endif // INCLUDE_WFC_H

// IMPLEMENTATION

#ifdef WFC_IMPLEMENTATION

#include <float.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef WFC_ASSERT
#include <assert.h>
#define WFC_ASSERT(ctx, cond) assert(cond)
#endif

#ifndef WFC_MALLOC
#define WFC_MALLOC(ctx, sz) malloc(sz)
#endif

#ifndef WFC_FREE
#define WFC_FREE(ctx, p) free(p)
#endif

// should return a float in [0, 1)
#ifndef WFC_RAND
#define WFC_RAND(ctx) wfc__rand()
#endif

// basic utility

int wfc__min_i(int a, int b) {
    return a < b ? a : b;
}

int wfc__max_i(int a, int b) {
    return a < b ? b : a;
}

bool wfc__approxEq_f(float a, float b) {
    const float absDiff = 0.001f;
    const float relDiff = FLT_EPSILON;

    if (fabsf(a - b) < absDiff) return true;

    if (fabsf(a) < fabsf(b)) return fabsf((a - b) / b) < relDiff;
    return fabsf((a - b) / a) < relDiff;
}

// rng utility

// [0, 1)
float wfc__rand(void) {
    return (float)rand() / ((float)RAND_MAX + 1.0f);
}

// [0, n)
int wfc__rand_i(void *ctx, int n) {
    (void)ctx;
    return (int)(WFC_RAND(ctx) * (float)n);
}

// array utility

int wfc__indWrap(int ind, int sz) {
    if (ind >= 0) return ind % sz;
    return sz + ind % sz;
}

#define WFC__A2D_DEF(type, abbrv) \
    struct wfc__A2d_##abbrv { \
        int d02, d12; \
        type *a; \
    }

#define WFC__A2D_LEN(arr) ((arr).d02 * (arr).d12)

#define WFC__A2D_SIZE(arr) ((size_t)WFC__A2D_LEN(arr) * sizeof(*(arr).a))

#define WFC__A2D_GET(arr, c0, c1) \
    ((arr).a[ \
        (c0) * (arr).d12 + \
        (c1)])

#define WFC__A2D_GET_WRAP(arr, c0, c1) \
    ((arr).a[ \
        wfc__indWrap(c0, (arr).d02) * (arr).d12 + \
        wfc__indWrap(c1, (arr).d12)])

#define WFC__A3D_DEF(type, abbrv) \
    struct wfc__A3d_##abbrv { \
        int d03, d13, d23; \
        type *a; \
    }

#define WFC__A3D_LEN(arr) ((arr).d03 * (arr).d13 * (arr).d23)

#define WFC__A3D_SIZE(arr) ((size_t)WFC__A3D_LEN(arr) * sizeof(*(arr).a))

#define WFC__A3D_GET(arr, c0, c1, c2) \
    ((arr).a[ \
        (c0) * (arr).d13 * (arr).d23 + \
        (c1) * (arr).d23 + \
        (c2)])

#define WFC__A3D_GET_WRAP(arr, c0, c1, c2) \
    ((arr).a[ \
        wfc__indWrap(c0, (arr).d03) * (arr).d13 * (arr).d23 + \
        wfc__indWrap(c1, (arr).d13) * (arr).d23 + \
        wfc__indWrap(c2, (arr).d23)])

#define WFC__A4D_DEF(type, abbrv) \
    struct wfc__A4d_##abbrv { \
        int d04, d14, d24, d34; \
        type *a; \
    }

#define WFC__A4D_LEN(arr) ((arr).d04 * (arr).d14 * (arr).d24 * (arr).d34)

#define WFC__A4D_SIZE(arr) ((size_t)WFC__A4D_LEN(arr) * sizeof(*(arr).a))

#define WFC__A4D_GET(arr, c0, c1, c2, c3) \
    ((arr).a[ \
        (c0) * (arr).d14 * (arr).d24 * (arr).d34 + \
        (c1) * (arr).d24 * (arr).d34 + \
        (c2) * (arr).d34 + \
        (c3)])

#define WFC__A4D_GET_WRAP(arr, c0, c1, c2, c3) \
    ((arr).a[ \
        wfc__indWrap(c0, (arr).d04) * (arr).d14 * (arr).d24 * (arr).d34 + \
        wfc__indWrap(c1, (arr).d14) * (arr).d24 * (arr).d34 + \
        wfc__indWrap(c2, (arr).d24) * (arr).d34 + \
        wfc__indWrap(c3, (arr).d34)])

void wfc__indToCoords2d(int d1, int ind, int *c0, int *c1) {
    int c0_ = ind / d1;
    ind -= c0_ * d1;
    int c1_ = ind;

    *c0 = c0_;
    *c1 = c1_;
}

// bools are not used in some places due to performance concerns
WFC__A2D_DEF(bool, b);
WFC__A2D_DEF(uint8_t, u8);
WFC__A2D_DEF(float, f);
WFC__A3D_DEF(uint8_t, u8);
WFC__A3D_DEF(const uint8_t, cu8);
WFC__A4D_DEF(uint8_t, u8);

// wfc code

// @TODO some combs are equivalent, so work in gathering patts gets duplicated
enum {
    wfc__tfFlipC0 = 1 << 0,
    wfc__tfFlipC1 = 1 << 1,
    wfc__tfRot90 = 1 << 2,
    wfc__tfRot180 = 1 << 3,
    wfc__tfRot270 = wfc__tfRot90 | wfc__tfRot180,

    wfc__tfCnt = 1 << 4
};

struct wfc__Pattern {
    int c0, c1;
    int tf;

    int edgeC0Lo, edgeC0Hi, edgeC1Lo, edgeC1Hi;
    int freq;
};

void wfc__coordsPattToSrc(
    int n,
    struct wfc__Pattern patt, int pC0, int pC1,
    int sD0, int sD1, int *sC0, int *sC1) {
    int tfC0 = pC0;
    int tfC1 = pC1;

    // map from patt space to src space
    {
        if (patt.tf & wfc__tfFlipC0) tfC0 = n - 1 - tfC0;
        if (patt.tf & wfc__tfFlipC1) tfC1 = n - 1 - tfC1;

        // rot270 is rot90 plus rot180 (both in bitmask and as transformation)
        if (patt.tf & wfc__tfRot90) {
            int tmpC0 = tfC0;
            int tmpC1 = tfC1;

            tfC0 = tmpC1;
            tfC1 = n - 1 - tmpC0;
        }
        if (patt.tf & wfc__tfRot180) {
            tfC0 = n - 1 - tfC0;
            tfC1 = n - 1 - tfC1;
        }
    }

    int sC0_ = wfc__indWrap(patt.c0 + tfC0, sD0);
    int sC1_ = wfc__indWrap(patt.c1 + tfC1, sD1);

    if (sC0 != NULL) *sC0 = sC0_;
    if (sC1 != NULL) *sC1 = sC1_;
}

void wfc__fillPattEdges(int n, int sD0, int sD1,
    struct wfc__Pattern *patt) {
    bool edgeC0Lo = patt->c0 == 0;
    bool edgeC0Hi = patt->c0 + n == sD0;
    bool edgeC1Lo = patt->c1 == 0;
    bool edgeC1Hi = patt->c1 + n == sD1;

    // map from src space to patt space
    {
        // rot270 is rot90 plus rot180 (both in bitmask and as transformation)
        if (patt->tf & wfc__tfRot180) {
            bool tmpC0Lo = edgeC0Lo;
            bool tmpC0Hi = edgeC0Hi;
            bool tmpC1Lo = edgeC1Lo;
            bool tmpC1Hi = edgeC1Hi;

            edgeC0Lo = tmpC0Hi;
            edgeC0Hi = tmpC0Lo;
            edgeC1Lo = tmpC1Hi;
            edgeC1Hi = tmpC1Lo;
        }
        if (patt->tf & wfc__tfRot90) {
            bool tmpC0Lo = edgeC0Lo;
            bool tmpC0Hi = edgeC0Hi;
            bool tmpC1Lo = edgeC1Lo;
            bool tmpC1Hi = edgeC1Hi;

            edgeC0Lo = tmpC1Hi;
            edgeC0Hi = tmpC1Lo;
            edgeC1Lo = tmpC0Lo;
            edgeC1Hi = tmpC0Hi;
        }

        if (patt->tf & wfc__tfFlipC1) {
            bool tmp = edgeC1Lo;
            edgeC1Lo = edgeC1Hi;
            edgeC1Hi = tmp;
        }
        if (patt->tf & wfc__tfFlipC0) {
            bool tmp = edgeC0Lo;
            edgeC0Lo = edgeC0Hi;
            edgeC0Hi = tmp;
        }
    }

    patt->edgeC0Lo = edgeC0Lo;
    patt->edgeC0Hi = edgeC0Hi;
    patt->edgeC1Lo = edgeC1Lo;
    patt->edgeC1Hi = edgeC1Hi;
}

void wfc__coordsDstToWave(
    int dC0, int dC1,
    const struct wfc__A3d_u8 wave,
    int *wC0, int *wC1,
    int *offC0, int *offC1) {
    int wC0_ = wfc__min_i(dC0, wave.d03 - 1);
    int wC1_ = wfc__min_i(dC1, wave.d13 - 1);

    int offC0_ = dC0 - wC0_;
    int offC1_ = dC1 - wC1_;

    if (wC0 != NULL) *wC0 = wC0_;
    if (wC1 != NULL) *wC1 = wC1_;
    if (offC0 != NULL) *offC0 = offC0_;
    if (offC1 != NULL) *offC1 = offC1_;
}

int wfc__pattCombCnt(int d0, int d1) {
    return d0 * d1 * wfc__tfCnt;
}

void wfc__indToPattComb(int d1, int ind, struct wfc__Pattern *patt) {
    wfc__indToCoords2d(d1, ind / wfc__tfCnt, &patt->c0, &patt->c1);
    patt->tf = ind % wfc__tfCnt;
}

bool wfc__satisfiesOptions(int n, int options, int sD0, int sD1,
    struct wfc__Pattern patt) {
    if ((patt.tf & wfc__tfFlipC0) && !(options & wfc_optFlipC0)) return false;
    if ((patt.tf & wfc__tfFlipC1) && !(options & wfc_optFlipC1)) return false;

    if ((patt.tf & (wfc__tfRot90 | wfc__tfRot180 | wfc__tfRot270)) &&
        !(options & wfc_optRotate)) {
        return false;
    }

    if ((options & wfc_optEdgeFixC0) && patt.c0 + n > sD0) return false;
    if ((options & wfc_optEdgeFixC1) && patt.c1 + n > sD1) return false;

    return true;
}

bool wfc__patternsEq(
    int n, const struct wfc__A3d_cu8 src,
    struct wfc__Pattern pattA, struct wfc__Pattern pattB) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            int sAC0, sAC1;
            wfc__coordsPattToSrc(n, pattA, i, j, src.d03, src.d13,
                &sAC0, &sAC1);

            int sBC0, sBC1;
            wfc__coordsPattToSrc(n, pattB, i, j, src.d03, src.d13,
                &sBC0, &sBC1);

            const uint8_t *pxA = &WFC__A3D_GET(src, sAC0, sAC1, 0);
            const uint8_t *pxB = &WFC__A3D_GET(src, sBC0, sBC1, 0);

            if (memcmp(pxA, pxB, (size_t)src.d23) != 0) return false;
        }
    }

    return true;
}

struct wfc__Pattern* wfc__gatherPatterns(
    void *ctx,
    int n, int options,
    const struct wfc__A3d_cu8 src,
    int *cnt) {
    (void)ctx;

    struct wfc__Pattern *patts = NULL;

    int pattCnt = 0;
    for (int i = 0; i < wfc__pattCombCnt(src.d03, src.d13); ++i) {
        struct wfc__Pattern patt = {0, 0, 0, 0, 0, 0, 0, 0};
        wfc__indToPattComb(src.d13, i, &patt);
        if (!wfc__satisfiesOptions(n, options, src.d03, src.d13, patt)) {
            continue;
        }

        bool seenBefore = false;
        for (int i1 = 0; !seenBefore && i1 < i; ++i1) {
            struct wfc__Pattern patt1 = {0, 0, 0, 0, 0, 0, 0, 0};
            wfc__indToPattComb(src.d13, i1, &patt1);
            if (!wfc__satisfiesOptions(n, options, src.d03, src.d13, patt1)) {
                continue;
            }

            if (wfc__patternsEq(n, src, patt, patt1)) seenBefore = true;
        }

        if (!seenBefore) ++pattCnt;
    }

    patts = (struct wfc__Pattern*)WFC_MALLOC(
        ctx, (size_t)pattCnt * sizeof(*patts));
    int pattInd = 0;
    for (int i = 0; i < wfc__pattCombCnt(src.d03, src.d13); ++i) {
        struct wfc__Pattern patt = {0, 0, 0, 0, 0, 0, 0, 0};
        wfc__indToPattComb(src.d13, i, &patt);
        if (!wfc__satisfiesOptions(n, options, src.d03, src.d13, patt)) {
            continue;
        }

        wfc__fillPattEdges(n, src.d03, src.d13, &patt);
        patt.freq = 1;

        bool seenBefore = false;
        for (int i1 = 0; !seenBefore && i1 < pattInd; ++i1) {
            if (wfc__patternsEq(n, src, patt, patts[i1])) {
                patts[i1].edgeC0Lo |= patt.edgeC0Lo;
                patts[i1].edgeC0Hi |= patt.edgeC0Hi;
                patts[i1].edgeC1Lo |= patt.edgeC1Lo;
                patts[i1].edgeC1Hi |= patt.edgeC1Hi;

                ++patts[i1].freq;

                seenBefore = true;
            }
        }

        if (!seenBefore) patts[pattInd++] = patt;
    }
    WFC_ASSERT(ctx, pattInd == pattCnt);

    *cnt = pattCnt;
    return patts;
}

bool wfc__overlapMatches(
    void *ctx,
    int n, const struct wfc__A3d_cu8 src,
    int dC0, int dC1,
    struct wfc__Pattern pattA, struct wfc__Pattern pattB) {
    (void)ctx;

    WFC_ASSERT(ctx, abs(dC0) < n);
    WFC_ASSERT(ctx, abs(dC1) < n);

    int overlapD0 = n - abs(dC0);
    int overlapD1 = n - abs(dC1);

    int overlapPAC0 = dC0 > 0 ? dC0 : 0;
    int overlapPAC1 = dC1 > 0 ? dC1 : 0;
    int overlapPBC0 = dC0 > 0 ? 0 : abs(dC0);
    int overlapPBC1 = dC1 > 0 ? 0 : abs(dC1);

    for (int i = 0; i < overlapD0; ++i) {
        for (int j = 0; j < overlapD1; ++j) {
            int pAC0 = overlapPAC0 + i;
            int pAC1 = overlapPAC1 + j;

            int sAC0, sAC1;
            wfc__coordsPattToSrc(n, pattA, pAC0, pAC1, src.d03, src.d13,
                &sAC0, &sAC1);

            int pBC0 = overlapPBC0 + i;
            int pBC1 = overlapPBC1 + j;

            int sBC0, sBC1;
            wfc__coordsPattToSrc(n, pattB, pBC0, pBC1, src.d03, src.d13,
                &sBC0, &sBC1);

            const uint8_t *pxA = &WFC__A3D_GET(src, sAC0, sAC1, 0);
            const uint8_t *pxB = &WFC__A3D_GET(src, sBC0, sBC1, 0);

            if (memcmp(pxA, pxB, (size_t)src.d23) != 0) return false;
        }
    }

    return true;
}

struct wfc__A4d_u8 wfc__calcOverlaps(
    void *ctx,
    int n, const struct wfc__A3d_cu8 src,
    int pattCnt, const struct wfc__Pattern *patts) {
    struct wfc__A4d_u8 overlaps;
    overlaps.d04 = 2 * n - 1;
    overlaps.d14 = 2 * n - 1;
    overlaps.d24 = pattCnt;
    overlaps.d34 = pattCnt;
    overlaps.a = (uint8_t*)WFC_MALLOC(ctx, WFC__A4D_SIZE(overlaps));

    for (int dC0 = -(n - 1); dC0 <= n - 1; ++dC0) {
        for (int dC1 = -(n - 1); dC1 <= n - 1; ++dC1) {
            for (int i = 0; i < pattCnt; ++i) {
                for (int j = 0; j < pattCnt; ++j) {
                    bool overlap = wfc__overlapMatches(ctx, n, src,
                        dC0, dC1, patts[i], patts[j]);
                    WFC__A4D_GET(overlaps, dC0 + n - 1, dC1 + n - 1, i, j) =
                        overlap ? 1 : 0;
                }
            }
        }
    }

    return overlaps;
}

bool wfc__restrictKept(
    int n,
    const struct wfc__A3d_cu8 src,
    const struct wfc__Pattern *patts,
    const struct wfc__A3d_cu8 dst,
    const struct wfc__A2d_b keep,
    struct wfc__A3d_u8 wave) {
    const int pattCnt = wave.d23, bytesPerPixel = dst.d23;

    bool modified = false;

    for (int wC0 = 0; wC0 < wave.d03; ++wC0) {
        for (int wC1 = 0; wC1 < wave.d13; ++wC1) {
            for (int i = 0; i < n; ++i) {
                for (int j = 0; j < n; ++j) {
                    int dC0 = wfc__indWrap(wC0 + i, dst.d03);
                    int dC1 = wfc__indWrap(wC1 + j, dst.d13);

                    if (!WFC__A2D_GET(keep, dC0, dC1)) continue;

                    const unsigned char *dPx = &WFC__A3D_GET(dst, dC0, dC1, 0);

                    for (int p = 0; p < pattCnt; ++p) {
                        if (!WFC__A3D_GET(wave, wC0, wC1, p)) continue;

                        int sC0, sC1;
                        wfc__coordsPattToSrc(
                            n,
                            patts[p], i, j,
                            src.d03, src.d13,
                            &sC0, &sC1);

                        const unsigned char *sPx =
                            &WFC__A3D_GET(src, sC0, sC1, 0);

                        if (memcmp(dPx, sPx, (size_t)bytesPerPixel) != 0) {
                            WFC__A3D_GET(wave, wC0, wC1, p) = 0;
                            modified = true;
                        }
                    }
                }
            }
        }
    }

    return modified;
}

bool wfc__restrictEdges(
    int options,
    const struct wfc__Pattern *patts,
    struct wfc__A3d_u8 wave) {
    const int d0 = wave.d03, d1 = wave.d13, pattCnt = wave.d23;

    bool modified = false;

    if (options & wfc_optEdgeFixC0) {
        for (int i = 0; i < d1; ++i) {
            for (int p = 0; p < pattCnt; ++p) {
                if (WFC__A3D_GET(wave, 0, i, p) && !patts[p].edgeC0Lo) {
                    WFC__A3D_GET(wave, 0, i, p) = 0;
                    modified = true;
                }
                if (WFC__A3D_GET(wave, d0 - 1, i, p) && !patts[p].edgeC0Hi) {
                    WFC__A3D_GET(wave, d0 - 1, i, p) = 0;
                    modified = true;
                }
            }
        }
    }
    if (options & wfc_optEdgeFixC1) {
        for (int i = 0; i < d0; ++i) {
            for (int p = 0; p < pattCnt; ++p) {
                if (WFC__A3D_GET(wave, i, 0, p) && !patts[p].edgeC1Lo) {
                    WFC__A3D_GET(wave, i, 0, p) = 0;
                    modified = true;
                }
                if (WFC__A3D_GET(wave, i, d1 - 1, p) && !patts[p].edgeC1Hi) {
                    WFC__A3D_GET(wave, i, d1 - 1, p) = 0;
                    modified = true;
                }
            }
        }
    }

    return modified;
}

void wfc__calcEntropies(
    const struct wfc__Pattern *patts,
    const struct wfc__A3d_u8 wave,
    struct wfc__A2d_f entropies) {
    for (int c0 = 0; c0 < wave.d03; ++c0) {
        for (int c1 = 0; c1 < wave.d13; ++c1) {
            int totalFreq = 0;
            int availPatts = 0;
            for (int p = 0; p < wave.d23; ++p) {
                if (WFC__A3D_GET(wave, c0, c1, p)) {
                    totalFreq += patts[p].freq;
                    ++availPatts;
                }
            }

            float entropy = 0.0f;
            // check is here to ensure entropy of observed points becomes 0
            if (availPatts > 1) {
                for (int p = 0; p < wave.d23; ++p) {
                    if (WFC__A3D_GET(wave, c0, c1, p)) {
                        float prob = (float)patts[p].freq / (float)totalFreq;
                        entropy -= prob * log2f(prob);
                    }
                }
            }

            WFC__A2D_GET(entropies, c0, c1) = entropy;
        }
    }
}

void wfc__observeOne(
    void *ctx,
    int pattCnt, const struct wfc__Pattern *patts,
    const struct wfc__A2d_f entropies,
    struct wfc__A3d_u8 wave, int *obsC0, int *obsC1) {
    float smallest = 0;
    int smallestCnt = 0;
    for (int i = 0; i < WFC__A2D_LEN(entropies); ++i) {
        // skip observed points
        if (entropies.a[i] == 0.0f) continue;

        if (smallestCnt > 0 && wfc__approxEq_f(entropies.a[i], smallest)) {
            ++smallestCnt;
        } else if (smallestCnt == 0 || entropies.a[i] < smallest) {
            smallest = entropies.a[i];
            smallestCnt = 1;
        }
    }

    int chosenC0, chosenC1;
    {
        int chosenPnt = 0;
        int chosenSmallestPnt = wfc__rand_i(ctx, smallestCnt);
        for (int i = 0; i < WFC__A2D_LEN(entropies); ++i) {
            if (wfc__approxEq_f(entropies.a[i], smallest)) {
                chosenPnt = i;
                if (chosenSmallestPnt == 0) break;
                --chosenSmallestPnt;
            }
        }
        wfc__indToCoords2d(entropies.d12, chosenPnt, &chosenC0, &chosenC1);
    }

    int chosenPatt = 0;
    {
        int totalFreq = 0;
        for (int i = 0; i < pattCnt; ++i) {
            if (WFC__A3D_GET(wave, chosenC0, chosenC1, i)) {
                totalFreq += patts[i].freq;
            }
        }
        int chosenInst = wfc__rand_i(ctx, totalFreq);

        for (int i = 0; i < pattCnt; ++i) {
            if (WFC__A3D_GET(wave, chosenC0, chosenC1, i)) {
                if (chosenInst < patts[i].freq) {
                    chosenPatt = i;
                    break;
                }
                chosenInst -= patts[i].freq;
            }
        }
    }

    *obsC0 = chosenC0;
    *obsC1 = chosenC1;
    for (int i = 0; i < pattCnt; ++i) {
        WFC__A3D_GET(wave, chosenC0, chosenC1, i) = 0;
    }
    WFC__A3D_GET(wave, chosenC0, chosenC1, chosenPatt) = 1;
}

bool wfc__propagateOntoDelta(
    int n, int nC0, int nC1, int dC0, int dC1,
    const struct wfc__A4d_u8 overlaps,
    struct wfc__A3d_u8 wave) {
    const int c0 = wfc__indWrap(nC0 + dC0, wave.d03);
    const int c1 = wfc__indWrap(nC1 + dC1, wave.d13);
    const int pattCnt = wave.d23;

    int oldAvailPattCnt = 0;
    for (int p = 0; p < pattCnt; ++p) {
        if (WFC__A3D_GET(wave, c0, c1, p)) ++oldAvailPattCnt;
    }

    for (int p = 0; p < pattCnt; ++p) {
        if (!WFC__A3D_GET(wave, c0, c1, p)) continue;

        uint8_t total = 0;
        for (int pN = 0; pN < pattCnt; ++pN) {
            total |= WFC__A3D_GET(wave, nC0, nC1, pN) &
                WFC__A4D_GET(overlaps, -dC0 + n - 1, -dC1 + n - 1, p, pN);
        }

        WFC__A3D_GET(wave, c0, c1, p) = total;
    }

    int newAvailPattCnt = 0;
    for (int p = 0; p < pattCnt; ++p) {
        if (WFC__A3D_GET(wave, c0, c1, p)) ++newAvailPattCnt;
    }

    return oldAvailPattCnt != newAvailPattCnt;
}

bool wfc__propagateOntoNeighbours(
    int n, int options,
    int nC0, int nC1,
    const struct wfc__A4d_u8 overlaps,
    struct wfc__A2d_u8 ripple,
    struct wfc__A3d_u8 wave) {
    bool modified = false;

    for (int dC0 = -(n - 1); dC0 <= n - 1; ++dC0) {
        for (int dC1 = -(n - 1); dC1 <= n - 1; ++dC1) {
            if (((options & wfc_optEdgeFixC0) &&
                    (nC0 + dC0 < 0 || nC0 + dC0 >= wave.d03)) ||
                ((options & wfc_optEdgeFixC1) &&
                    (nC1 + dC1 < 0 || nC1 + dC1 >= wave.d13))) {
                continue;
            }

            if (wfc__propagateOntoDelta(n, nC0, nC1, dC0, dC1,
                    overlaps, wave)) {
                WFC__A2D_GET_WRAP(ripple, nC0 + dC0, nC1 + dC1) = 1;
                modified = true;
            }
        }
    }

    return modified;
}

void wfc__propagateFromRipple(
    int n, int options,
    const struct wfc__A4d_u8 overlaps,
    struct wfc__A2d_u8 ripple,
    struct wfc__A3d_u8 wave) {
    bool modified = true;
    while (modified) {
        modified = false;

        for (int nC0 = 0; nC0 < wave.d03; ++nC0) {
            for (int nC1 = 0; nC1 < wave.d13; ++nC1) {
                if (!WFC__A2D_GET(ripple, nC0, nC1)) continue;

                if (wfc__propagateOntoNeighbours(n, options, nC0, nC1,
                        overlaps, ripple, wave)) {
                    modified = true;
                }

                WFC__A2D_GET(ripple, nC0, nC1) = 0;
            }
        }
    }
}

void wfc__propagateFromAll(
    int n, int options,
    const struct wfc__A4d_u8 overlaps,
    struct wfc__A2d_u8 ripple,
    struct wfc__A3d_u8 wave) {
    memset(ripple.a, 1, WFC__A2D_SIZE(ripple));

    wfc__propagateFromRipple(n, options, overlaps, ripple, wave);
}

void wfc__propagateFromSeed(
    int n, int options,
    int seedC0, int seedC1,
    const struct wfc__A4d_u8 overlaps,
    struct wfc__A2d_u8 ripple,
    struct wfc__A3d_u8 wave) {
    memset(ripple.a, 0, WFC__A2D_SIZE(ripple));
    WFC__A2D_GET(ripple, seedC0, seedC1) = 1;

    wfc__propagateFromRipple(n, options, overlaps, ripple, wave);
}

int wfc__calcStatus(const struct wfc__A3d_u8 wave) {
    int minPatts = wave.d23, maxPatts = 0;
    for (int c0 = 0; c0 < wave.d03; ++c0) {
        for (int c1 = 0; c1 < wave.d13; ++c1) {
            int cntPatts = 0;
            for (int p = 0; p < wave.d23; ++p) {
                if (WFC__A3D_GET(wave, c0, c1, p)) ++cntPatts;
            }

            if (cntPatts < minPatts) minPatts = cntPatts;
            if (cntPatts > maxPatts) maxPatts = cntPatts;
        }
    }

    if (minPatts == 0) {
        // contradiction reached
        return wfc_failed;
    }
    if (maxPatts == 1) {
        return wfc_completed;
    }
    // still in progress
    return 0;
}

struct wfc_State {
    int status;
    void *ctx;
    int n, options, bytesPerPixel;
    int srcD0, srcD1, dstD0, dstD1;
    struct wfc__Pattern *patts;
    struct wfc__A4d_u8 overlaps;
    struct wfc__A3d_u8 wave;
    struct wfc__A2d_f entropies;
    struct wfc__A2d_u8 ripple;
};

int wfc_generate(
    int n, int options, int bytesPerPixel,
    int srcW, int srcH, const unsigned char *src,
    int dstW, int dstH, unsigned char *dst) {
    return wfc_generateEx(
        n, options, bytesPerPixel,
        srcW, srcH, src,
        dstW, dstH, dst,
        NULL, NULL
    );
}

int wfc_generateEx(
    int n, int options, int bytesPerPixel,
    int srcW, int srcH, const unsigned char *src,
    int dstW, int dstH, unsigned char *dst,
    void *ctx,
    bool *keep) {
    int ret = 0;

    wfc_State *state = wfc_initEx(n, options, bytesPerPixel,
        srcW, srcH, src, dstW, dstH, dst, ctx, keep);
    if (state == NULL) return wfc_callerError;

    while (!wfc_step(state));

    if (wfc_status(state) < 0) {
        ret = wfc_status(state);
    } else if (wfc_status(state) == wfc_completed) {
        int code = wfc_blit(state, src, dst);
        if (code != 0) ret = code;
    }

    wfc_free(state);

    return ret;
}

wfc_State* wfc_init(
    int n, int options, int bytesPerPixel,
    int srcW, int srcH, const unsigned char *src,
    int dstW, int dstH) {
    return wfc_initEx(
        n, options, bytesPerPixel,
        srcW, srcH, src,
        dstW, dstH, NULL,
        NULL, NULL
    );
}

wfc_State* wfc_initEx(
    int n, int options, int bytesPerPixel,
    int srcW, int srcH, const unsigned char *src,
    int dstW, int dstH, const unsigned char *dst,
    void *ctx,
    bool *keep) {
    if (n <= 0 ||
        bytesPerPixel <= 0 ||
        srcW <= 0 || srcH <= 0 || src == NULL ||
        dstW <= 0 || dstH <= 0 ||
        n > srcW || n > srcH || n > dstW || n > dstH) {
        return NULL;
    }
    if (keep != NULL && dst == NULL) {
        return NULL;
    }

    struct wfc__A3d_cu8 srcA = {srcH, srcW, bytesPerPixel, src};

    wfc_State *state = (wfc_State*)WFC_MALLOC(ctx, sizeof(*state));

    state->status = 0;
    state->ctx = ctx;
    state->n = n;
    state->options = options;
    state->bytesPerPixel = bytesPerPixel;
    state->srcD0 = srcH;
    state->srcD1 = srcW;
    state->dstD0 = dstH;
    state->dstD1 = dstW;

    int pattCnt;
    state->patts = wfc__gatherPatterns(ctx, n, options, srcA, &pattCnt);

    state->overlaps = wfc__calcOverlaps(ctx, n, srcA, pattCnt, state->patts);

    state->wave.d03 = dstH;
    if (options & wfc_optEdgeFixC0) state->wave.d03 -= n - 1;
    state->wave.d13 = dstW;
    if (options & wfc_optEdgeFixC1) state->wave.d13 -= n - 1;
    state->wave.d23 = pattCnt;
    state->wave.a = (uint8_t*)WFC_MALLOC(ctx, WFC__A3D_SIZE(state->wave));
    for (int i = 0; i < WFC__A3D_LEN(state->wave); ++i) state->wave.a[i] = 1;

    state->entropies.d02 = state->wave.d03;
    state->entropies.d12 = state->wave.d13;
    state->entropies.a = (float*)WFC_MALLOC(
        ctx, WFC__A2D_SIZE(state->entropies));

    state->ripple.d02 = state->wave.d03;
    state->ripple.d12 = state->wave.d13;
    state->ripple.a = (uint8_t*)WFC_MALLOC(ctx, WFC__A2D_SIZE(state->ripple));

    bool propagate = false;

    if (keep != NULL) {
        struct wfc__A3d_cu8 dstA =
            {state->dstD0, state->dstD1, state->bytesPerPixel, dst};
        struct wfc__A2d_b keepA = {state->dstD0, state->dstD1, keep};

        if (wfc__restrictKept(
                n, srcA, state->patts, dstA, keepA, state->wave)) {
            propagate = true;
        }
    }

    if (options & (wfc_optEdgeFixC0 | wfc_optEdgeFixC1)) {
        if (wfc__restrictEdges(options, state->patts, state->wave)) {
            propagate = true;
        }
    }

    if (propagate) {
        wfc__propagateFromAll(n, options,
            state->overlaps, state->ripple, state->wave);
        state->status = wfc__calcStatus(state->wave);
    }

    return state;
}

int wfc_status(const wfc_State *state) {
    if (state == NULL) return wfc_callerError;

    return state->status;
}

int wfc_step(wfc_State *state) {
    if (state == NULL) return wfc_callerError;

    if (state->status != 0) return state->status;

    int pattCnt = state->wave.d23;

    wfc__calcEntropies(state->patts, state->wave, state->entropies);

    int obsC0, obsC1;
    wfc__observeOne(
        state->ctx, pattCnt, state->patts, state->entropies, state->wave,
        &obsC0, &obsC1);

    wfc__propagateFromSeed(state->n, state->options,
        obsC0, obsC1,
        state->overlaps, state->ripple, state->wave);

    state->status = wfc__calcStatus(state->wave);

    return state->status;
}

int wfc_blit(
    const wfc_State *state,
    const unsigned char *src, unsigned char *dst) {
    if (state == NULL || state->status != wfc_completed ||
        src == NULL || dst == NULL) {
        return wfc_callerError;
    }

    struct wfc__A3d_cu8 srcA =
        {state->srcD0, state->srcD1, state->bytesPerPixel, src};
    struct wfc__A3d_u8 dstA =
        {state->dstD0, state->dstD1, state->bytesPerPixel, dst};

    int pattCnt = state->wave.d23;

    for (int c0 = 0; c0 < state->dstD0; ++c0) {
        for (int c1 = 0; c1 < state->dstD1; ++c1) {
            int wC0, wC1, pC0, pC1;
            wfc__coordsDstToWave(c0, c1, state->wave, &wC0, &wC1, &pC0, &pC1);

            int patt = 0;
            for (int p = 0; p < pattCnt; ++p) {
                if (WFC__A3D_GET(state->wave, wC0, wC1, p)) {
                    patt = p;
                    break;
                }
            }

            int sC0, sC1;
            wfc__coordsPattToSrc(
                state->n, state->patts[patt], pC0, pC1, srcA.d03, srcA.d13,
                &sC0, &sC1);

            const uint8_t *srcPx = &WFC__A3D_GET(srcA, sC0, sC1, 0);
            uint8_t *dstPx = &WFC__A3D_GET(dstA, c0, c1, 0);

            memcpy(dstPx, srcPx, (size_t)state->bytesPerPixel);
        }
    }

    return 0;
}

wfc_State* wfc_clone(const wfc_State *state) {
    if (state == NULL) return NULL;

    wfc_State *clone = (wfc_State*)WFC_MALLOC(state->ctx, sizeof(*clone));

    *clone = *state;

    int pattCnt = state->wave.d23;
    size_t pattsSz = (size_t)pattCnt * sizeof(*state->patts);
    clone->patts = (struct wfc__Pattern*)WFC_MALLOC(state->ctx, pattsSz);
    memcpy(clone->patts, state->patts, pattsSz);

    clone->overlaps.a = (uint8_t*)WFC_MALLOC(
        state->ctx, WFC__A4D_SIZE(state->overlaps));
    memcpy(clone->overlaps.a, state->overlaps.a,
        WFC__A4D_SIZE(state->overlaps));

    clone->wave.a = (uint8_t*)WFC_MALLOC(
        state->ctx, WFC__A3D_SIZE(state->wave));
    memcpy(clone->wave.a, state->wave.a,
        WFC__A3D_SIZE(state->wave));

    clone->entropies.a = (float*)WFC_MALLOC(
        state->ctx, WFC__A2D_SIZE(state->entropies));
    memcpy(clone->entropies.a, state->entropies.a,
        WFC__A2D_SIZE(state->entropies));

    clone->ripple.a = (uint8_t*)WFC_MALLOC(
        state->ctx, WFC__A2D_SIZE(state->ripple));
    memcpy(clone->ripple.a, state->ripple.a,
        WFC__A2D_SIZE(state->ripple));

    return clone;
}

void wfc_free(wfc_State *state) {
    if (state == NULL) return;

    void *ctx = state->ctx;
    (void)ctx;

    WFC_FREE(ctx, state->ripple.a);
    WFC_FREE(ctx, state->entropies.a);
    WFC_FREE(ctx, state->wave.a);
    WFC_FREE(ctx, state->overlaps.a);
    WFC_FREE(ctx, state->patts);
    WFC_FREE(ctx, state);
}

int wfc_patternCount(const wfc_State *state) {
    if (state == NULL) return wfc_callerError;

    return state->wave.d23;
}

int wfc_patternAvailable(const wfc_State *state, int patt, int x, int y) {
    if (state == NULL ||
        patt < 0 || patt >= state->wave.d23 ||
        x < 0 || x >= state->dstD1 ||
        y < 0 || y >= state->dstD0) {
        return wfc_callerError;
    }

    int wC0, wC1;
    wfc__coordsDstToWave(y, x, state->wave, &wC0, &wC1, NULL, NULL);

    return WFC__A3D_GET(state->wave, wC0, wC1, patt);
}

const unsigned char* wfc_pixelToBlit(const wfc_State *state,
    int patt, int x, int y, const unsigned char *src) {
    if (state == NULL ||
        patt < 0 || patt >= state->wave.d23 ||
        x < 0 || x >= state->dstD1 ||
        y < 0 || y >= state->dstD0 ||
        src == NULL) {
        return NULL;
    }

    struct wfc__A3d_cu8 srcA =
        {state->srcD0, state->srcD1, state->bytesPerPixel, src};

    int pC0, pC1;
    wfc__coordsDstToWave(y, x, state->wave, NULL, NULL, &pC0, &pC1);

    int sC0, sC1;
    wfc__coordsPattToSrc(
        state->n, state->patts[patt], pC0, pC1, srcA.d03, srcA.d13,
        &sC0, &sC1);

    return &WFC__A3D_GET(srcA, sC0, sC1, 0);
}

#endif // WFC_IMPLEMENTATION
