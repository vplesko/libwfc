#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// @TODO C++ support
// @TODO allow users to supply their own assert
// @TODO allow users to supply their own malloc et al.
// @TODO allow users to supply their own rand
// @TODO add restrict where appropriate

// public declarations

// @TODO input validation

enum {
    wfc_optHFlip = 0x2,
    wfc_optVFlip = 0x1,
};

typedef struct wfc_State wfc_State;

int wfc_generate(
    int n, int options, int bytesPerPixel,
    int srcW, int srcH, const unsigned char *src,
    int dstW, int dstH, unsigned char *dst);

wfc_State* wfc_init(
    int n, int options, int bytesPerPixel,
    int srcW, int srcH, const unsigned char *src,
    int dstW, int dstH);

wfc_State* wfc_clone(const wfc_State *state);

int wfc_patternCount(const wfc_State *state);

int wfc_patternPresent(const wfc_State *state, int patt, int x, int y);

const unsigned char* wfc_pixelToRender(const wfc_State *state,
    int patt, const unsigned char *src);

int wfc_done(wfc_State *state);

int wfc_status(wfc_State *state);

void wfc_step(wfc_State *state);

void wfc_render(
    const wfc_State *state,
    const unsigned char *src, unsigned char *dst);

void wfc_free(wfc_State *state);

// basic utility

int wfc__min_i(int a, int b) {
    return a < b ? a : b;
}

int wfc__max_i(int a, int b) {
    return a < b ? b : a;
}

int wfc__approxEq_f(float a, float b) {
    const float absDiff = 0.001f;
    const float relDiff = FLT_EPSILON;

    if (fabsf(a - b) < absDiff) return 1;

    if (fabsf(a) < fabsf(b)) return fabsf((a - b) / b) < relDiff;
    return fabsf((a - b) / a) < relDiff;
}

// rng utility

// [0, 1)
float wfc__rand(void) {
    return (float)rand() / ((float)RAND_MAX + 1.0f);
}

// [0, n)
int wfc__rand_i(int n) {
    return (int)(wfc__rand() * (float)n);
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

WFC__A2D_DEF(uint8_t, u8);
WFC__A2D_DEF(float, f);
WFC__A3D_DEF(uint8_t, u8);
WFC__A3D_DEF(const uint8_t, cu8);
WFC__A4D_DEF(uint8_t, u8);

// wfc code

enum {
    wfc__tfC0Flip = 0x1,
    wfc__tfC1Flip = 0x2,
    wfc__tfCnt = 0x4,
};

struct wfc__Pattern {
    int c0, c1;
    int tf;
    int freq;
};

void wfc__coordsPattToSrc(
    int n,
    struct wfc__Pattern patt, int pC0, int pC1,
    int *sC0, int *sC1) {
    int sC0_ = patt.c0 + (patt.tf & wfc__tfC0Flip ? n - 1 - pC0 : pC0);
    int sC1_ = patt.c1 + (patt.tf & wfc__tfC1Flip ? n - 1 - pC1 : pC1);

    if (sC0 != NULL) *sC0 = sC0_;
    if (sC1 != NULL) *sC1 = sC1_;
}

int wfc__pattCombCnt(int d0, int d1) {
    return d0 * d1 * wfc__tfCnt;
}

void wfc__indToPattComb(int d1, int ind, struct wfc__Pattern *patt) {
    wfc__indToCoords2d(d1, ind / wfc__tfCnt, &patt->c0, &patt->c1);
    patt->tf = ind % wfc__tfCnt;
}

int wfc__usesOnlyAllowedOptions(int tf, int options) {
    if ((tf & wfc__tfC0Flip) && !(options & wfc_optVFlip)) return 0;
    if ((tf & wfc__tfC1Flip) && !(options & wfc_optHFlip)) return 0;
    return 1;
}

int wfc__patternsEq(
    int n, const struct wfc__A3d_cu8 src,
    struct wfc__Pattern pattA, struct wfc__Pattern pattB) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            int sAC0, sAC1;
            wfc__coordsPattToSrc(n, pattA, i, j, &sAC0, &sAC1);

            int sBC0, sBC1;
            wfc__coordsPattToSrc(n, pattB, i, j, &sBC0, &sBC1);

            const uint8_t *pxA = &WFC__A3D_GET_WRAP(src, sAC0, sAC1, 0);
            const uint8_t *pxB = &WFC__A3D_GET_WRAP(src, sBC0, sBC1, 0);

            if (memcmp(pxA, pxB, (size_t)src.d23) != 0) return 0;
        }
    }
    return 1;
}

// @TODO test number of patterns when the API is introduced for that
struct wfc__Pattern* wfc__gatherPatterns(
    int n, int options, const struct wfc__A3d_cu8 src, int *cnt) {
    struct wfc__Pattern *patts = NULL;

    int pattCnt = 0;
    for (int i = 0; i < wfc__pattCombCnt(src.d03, src.d13); ++i) {
        struct wfc__Pattern patt = {0};
        wfc__indToPattComb(src.d13, i, &patt);
        if (!wfc__usesOnlyAllowedOptions(patt.tf, options)) continue;

        int seenBefore = 0;
        for (int i1 = 0; !seenBefore && i1 < i; ++i1) {
            struct wfc__Pattern patt1 = {0};
            wfc__indToPattComb(src.d13, i1, &patt1);
            if (!wfc__usesOnlyAllowedOptions(patt1.tf, options)) continue;

            if (wfc__patternsEq(n, src, patt, patt1)) seenBefore = 1;
        }

        if (!seenBefore) ++pattCnt;
    }

    patts = malloc((size_t)pattCnt * sizeof(*patts));
    int pattInd = 0;
    for (int i = 0; i < wfc__pattCombCnt(src.d03, src.d13); ++i) {
        struct wfc__Pattern patt = {0};
        wfc__indToPattComb(src.d13, i, &patt);
        patt.freq = 1;
        if (!wfc__usesOnlyAllowedOptions(patt.tf, options)) continue;

        int seenBefore = 0;
        for (int i1 = 0; !seenBefore && i1 < pattInd; ++i1) {
            if (wfc__patternsEq(n, src, patt, patts[i1])) {
                ++patts[i1].freq;
                seenBefore = 1;
            }
        }

        if (!seenBefore) patts[pattInd++] = patt;
    }
    assert(pattInd == pattCnt);

    *cnt = pattCnt;
    return patts;
}

int wfc__overlapMatches(
    int n, const struct wfc__A3d_cu8 src,
    int dC0, int dC1, struct wfc__Pattern pattA, struct wfc__Pattern pattB) {
    assert(abs(dC0) < n);
    assert(abs(dC1) < n);

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
            wfc__coordsPattToSrc(n, pattA, pAC0, pAC1, &sAC0, &sAC1);

            int pBC0 = overlapPBC0 + i;
            int pBC1 = overlapPBC1 + j;

            int sBC0, sBC1;
            wfc__coordsPattToSrc(n, pattB, pBC0, pBC1, &sBC0, &sBC1);

            const uint8_t *pxA = &WFC__A3D_GET_WRAP(src, sAC0, sAC1, 0);
            const uint8_t *pxB = &WFC__A3D_GET_WRAP(src, sBC0, sBC1, 0);

            if (memcmp(pxA, pxB, (size_t)src.d23) != 0) return 0;
        }
    }

    return 1;
}

struct wfc__A4d_u8 wfc__calcOverlaps(
    int n, const struct wfc__A3d_cu8 src,
    int pattCnt, const struct wfc__Pattern *patts) {
    struct wfc__A4d_u8 overlaps;
    overlaps.d04 = 2 * n - 1;
    overlaps.d14 = 2 * n - 1;
    overlaps.d24 = pattCnt;
    overlaps.d34 = pattCnt;
    overlaps.a = malloc(WFC__A4D_SIZE(overlaps));

    for (int dC0 = -(n - 1); dC0 <= n - 1; ++dC0) {
        for (int dC1 = -(n - 1); dC1 <= n - 1; ++dC1) {
            for (int i = 0; i < pattCnt; ++i) {
                for (int j = 0; j < pattCnt; ++j) {
                    int overlap = wfc__overlapMatches(n, src,
                        dC0, dC1, patts[i], patts[j]);
                    WFC__A4D_GET(overlaps, dC0 + n - 1, dC1 + n - 1, i, j) =
                        overlap ? 1 : 0;
                }
            }
        }
    }

    return overlaps;
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
    int pattCnt, const struct wfc__Pattern *patts,
    const struct wfc__A2d_f entropies,
    struct wfc__A3d_u8 wave, int *obsC0, int *obsC1) {
    float smallest;
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
        int chosenSmallestPnt = wfc__rand_i(smallestCnt);
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
        int chosenInst = wfc__rand_i(totalFreq);

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

int wfc__propagateOnto(
    int n, int nC0, int nC1, int dC0, int dC1,
    const struct wfc__A4d_u8 overlaps,
    struct wfc__A3d_u8 wave) {
    int c0 = wfc__indWrap(nC0 + dC0, wave.d03);
    int c1 = wfc__indWrap(nC1 + dC1, wave.d13);
    int pattCnt = wave.d23;

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

int wfc__propagateNeighbours(
    int n, int nC0, int nC1,
    const struct wfc__A4d_u8 overlaps,
    struct wfc__A2d_u8 ripple,
    struct wfc__A3d_u8 wave) {
    int modified = 0;

    for (int dC0 = -(n - 1); dC0 <= n - 1; ++dC0) {
        for (int dC1 = -(n - 1); dC1 <= n - 1; ++dC1) {
            if (wfc__propagateOnto(n, nC0, nC1, dC0, dC1, overlaps, wave)) {
                WFC__A2D_GET_WRAP(ripple, nC0 + dC0, nC1 + dC1) = 1;
                modified = 1;
            }
        }
    }

    return modified;
}

void wfc__propagate(
    int n, int seedC0, int seedC1,
    const struct wfc__A4d_u8 overlaps,
    struct wfc__A2d_u8 ripple,
    struct wfc__A3d_u8 wave) {
    memset(ripple.a, 0, WFC__A2D_SIZE(ripple));
    WFC__A2D_GET(ripple, seedC0, seedC1) = 1;

    int modified = 1;
    while (modified) {
        modified = 0;

        for (int nC0 = 0; nC0 < wave.d03; ++nC0) {
            for (int nC1 = 0; nC1 < wave.d13; ++nC1) {
                if (!WFC__A2D_GET(ripple, nC0, nC1)) continue;

                if (wfc__propagateNeighbours(n, nC0, nC1,
                        overlaps, ripple, wave)) {
                    modified = 1;
                }

                WFC__A2D_GET(ripple, nC0, nC1) = 0;
            }
        }
    }
}

struct wfc_State {
    int status;
    int n, bytesPerPixel;
    int sD0, sD1;
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
    int ret = 0;

    wfc_State *state = wfc_init(n, options, bytesPerPixel,
        srcW, srcH, src, dstW, dstH);

    while (!wfc_done(state)) wfc_step(state);

    if (wfc_status(state) < 0) {
        ret = -1;
    } else if (wfc_status(state) > 0) {
        wfc_render(state, src, dst);
    }

    wfc_free(state);

    return ret;
}

wfc_State* wfc_init(
    int n, int options, int bytesPerPixel,
    int srcW, int srcH, const unsigned char *src,
    int dstW, int dstH) {
    // @TODO instead, return a code for client error
    assert(n > 0);
    assert(bytesPerPixel > 0);
    assert(n <= srcW);
    assert(n <= srcH);
    assert(n <= dstW);
    assert(n <= dstH);
    assert(srcW > 0);
    assert(srcH > 0);
    assert(src != NULL);
    assert(dstW > 0);
    assert(dstH > 0);

    struct wfc__A3d_cu8 srcA = {srcH, srcW, bytesPerPixel, src};

    wfc_State *state = malloc(sizeof(*state));

    state->status = 0;
    state->n = n;
    state->bytesPerPixel = bytesPerPixel;
    state->sD0 = srcA.d03;
    state->sD1 = srcA.d13;

    int pattCnt;
    state->patts = wfc__gatherPatterns(n, options, srcA, &pattCnt);

    state->overlaps = wfc__calcOverlaps(n, srcA, pattCnt, state->patts);

    state->wave.d03 = dstH;
    state->wave.d13 = dstW;
    state->wave.d23 = pattCnt;
    state->wave.a = malloc(WFC__A3D_SIZE(state->wave));
    for (int i = 0; i < WFC__A3D_LEN(state->wave); ++i) state->wave.a[i] = 1;

    state->entropies.d02 = dstH;
    state->entropies.d12 = dstW;
    state->entropies.a = malloc(WFC__A2D_SIZE(state->entropies));

    state->ripple.d02 = dstH;
    state->ripple.d12 = dstW;
    state->ripple.a = malloc(WFC__A2D_SIZE(state->ripple));

    return state;
}

wfc_State* wfc_clone(const wfc_State *state) {
    wfc_State *clone = malloc(sizeof(*clone));

    *clone = *state;

    int pattCnt = state->wave.d23;
    size_t pattsSz = (size_t)pattCnt * sizeof(*state->patts);
    clone->patts = malloc(pattsSz);
    memcpy(clone->patts, state->patts, pattsSz);

    clone->overlaps.a = malloc(WFC__A4D_SIZE(state->overlaps));
    memcpy(clone->overlaps.a, state->overlaps.a,
        WFC__A4D_SIZE(state->overlaps));

    clone->wave.a = malloc(WFC__A3D_SIZE(state->wave));
    memcpy(clone->wave.a, state->wave.a,
        WFC__A3D_SIZE(state->wave));

    clone->entropies.a = malloc(WFC__A2D_SIZE(state->entropies));
    memcpy(clone->entropies.a, state->entropies.a,
        WFC__A2D_SIZE(state->entropies));

    clone->ripple.a = malloc(WFC__A2D_SIZE(state->ripple));
    memcpy(clone->ripple.a, state->ripple.a,
        WFC__A2D_SIZE(state->ripple));

    return clone;
}

int wfc_patternCount(const wfc_State *state) {
    return state->wave.d23;
}

int wfc_patternPresent(const wfc_State *state, int patt, int x, int y) {
    return WFC__A3D_GET(state->wave, y, x, patt);
}

const unsigned char* wfc_pixelToRender(const wfc_State *state,
    int patt, const unsigned char *src) {
    struct wfc__A3d_cu8 srcA =
        {state->sD0, state->sD1, state->bytesPerPixel, src};

    int sC0, sC1;
    wfc__coordsPattToSrc(state->n, state->patts[patt], 0, 0, &sC0, &sC1);

    return &WFC__A3D_GET_WRAP(srcA, sC0, sC1, 0);
}

int wfc_done(wfc_State *state) {
    return state->status != 0;
}

int wfc_status(wfc_State *state) {
    return state->status;
}

void wfc_step(wfc_State *state) {
    int pattCnt = state->wave.d23;

    {
        int minPatts = pattCnt, maxPatts = 0;
        for (int c0 = 0; c0 < state->wave.d03; ++c0) {
            for (int c1 = 0; c1 < state->wave.d13; ++c1) {
                int cntPatts = 0;
                for (int p = 0; p < pattCnt; ++p) {
                    if (WFC__A3D_GET(state->wave, c0, c1, p)) ++cntPatts;
                }

                if (cntPatts < minPatts) minPatts = cntPatts;
                if (cntPatts > maxPatts) maxPatts = cntPatts;
            }
        }

        if (minPatts == 0) {
            // contradiction reached
            state->status = -1;
            return;
        } else if (maxPatts == 1) {
            // WFC completed
            state->status = 1;
            return;
        }
    }

    wfc__calcEntropies(state->patts, state->wave, state->entropies);

    int obsC0, obsC1;
    wfc__observeOne(pattCnt, state->patts, state->entropies, state->wave,
        &obsC0, &obsC1);

    wfc__propagate(state->n, obsC0, obsC1, state->overlaps,
        state->ripple, state->wave);
}

void wfc_render(
    const wfc_State *state,
    const unsigned char *src, unsigned char *dst) {
    assert(dst != NULL);

    struct wfc__A3d_cu8 srcA =
        {state->sD0, state->sD1, state->bytesPerPixel, src};
    struct wfc__A3d_u8 dstA =
        {state->wave.d03, state->wave.d13, state->bytesPerPixel, dst};

    int pattCnt = state->wave.d23;

    for (int c0 = 0; c0 < state->wave.d03; ++c0) {
        for (int c1 = 0; c1 < state->wave.d13; ++c1) {
            int patt = 0;
            for (int p = 0; p < pattCnt; ++p) {
                if (WFC__A3D_GET(state->wave, c0, c1, p)) {
                    patt = p;
                    break;
                }
            }

            int sC0, sC1;
            wfc__coordsPattToSrc(state->n, state->patts[patt], 0, 0,
                &sC0, &sC1);

            const uint8_t *srcPx = &WFC__A3D_GET_WRAP(srcA, sC0, sC1, 0);
            uint8_t *dstPx = &WFC__A3D_GET(dstA, c0, c1, 0);

            memcpy(dstPx, srcPx, (size_t)state->bytesPerPixel);
        }
    }
}

void wfc_free(wfc_State *state) {
    free(state->ripple.a);
    free(state->entropies.a);
    free(state->wave.a);
    free(state->overlaps.a);
    free(state->patts);
    free(state);
}
