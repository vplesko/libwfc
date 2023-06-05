#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// @TODO allow users to supply their own assert
// @TODO allow users to supply their own malloc et al.
// @TODO allow users to supply their own rand
// @TODO add restrict where appropriate

// basic utility

#define WFC__ARR_LEN(a) ((int)(sizeof(a) / sizeof((a)[0])))

int wfc__product(int n, int *arr) {
    int prod = 1;
    for (int i = 0; i < n; ++i) prod *= arr[i];
    return prod;
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

// multi-dimensional array utility
// @TODO this is making the code ~2.5x slower!!!

int wfc__indWrap(int ind, int sz) {
    if (ind >= 0) return ind % sz;
    return sz + ind % sz;
}

int wfc__coordsToInd(int wrap, int rank, int *dim, ...) {
    va_list vargs;
    va_start(vargs, dim);

    int mul = 1, ind = 0;
    for (int i = 0; i < rank; ++i) {
        int coord = va_arg(vargs, int);
        if (wrap) coord = wfc__indWrap(coord, dim[i]);

        ind += coord * mul;
        mul *= dim[i];
    }

    return ind;
}

#define WFC__MDA_DEF(rank, type, abbrv) \
    struct wfc__Mda_##rank##abbrv { \
        type *a; \
        int dim[rank]; \
    }

#define WFC__MDA_RANK(mda) WFC__ARR_LEN((mda).dim)

#define WFC__MDA_LEN(mda) wfc__product(WFC__MDA_RANK(mda), (mda).dim)

#define WFC__MDA_SIZE(mda) ((size_t)WFC__MDA_LEN(mda) * sizeof(*(mda).a))

#define WFC__MDA_GET(mda, ...) \
    ((mda).a[wfc__coordsToInd(0, WFC__MDA_RANK(mda), (mda).dim, __VA_ARGS__)])

#define WFC__MDA_GET_WRAP(mda, ...) \
    ((mda).a[wfc__coordsToInd(1, WFC__MDA_RANK(mda), (mda).dim, __VA_ARGS__)])

void wfc__indToCoords2d(int dim0, int ind, int *c0, int *c1) {
    int c1_ = ind / dim0;
    ind -= c1_ * dim0;
    int c0_ = ind;

    *c0 = c0_;
    *c1 = c1_;
}

void wfc__indToCoords3d(
    int dim0, int dim1, int ind, int *c0, int *c1, int *c2) {
    int c2_ = ind / (dim0 * dim1);
    ind -= c2_ * (dim0 * dim1);
    int c1_ = ind / dim0;
    ind -= c1_ * dim0;
    int c0_ = ind;

    *c0 = c0_;
    *c1 = c1_;
    *c2 = c2_;
}

// wfc code

WFC__MDA_DEF(2, uint32_t, u32);
WFC__MDA_DEF(2, const uint32_t, cu32);
WFC__MDA_DEF(2, uint8_t, u8);
WFC__MDA_DEF(2, float, f);
WFC__MDA_DEF(3, uint8_t, u8);

struct wfc__Pattern {
    int l, t;
    int freq;
};

int wfc__patternsEq(int n, struct wfc__Mda_2cu32 srcM,
    struct wfc__Pattern patt1, struct wfc__Pattern patt2) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            uint32_t a = WFC__MDA_GET_WRAP(srcM, patt1.l + i, patt1.t + j);
            uint32_t b = WFC__MDA_GET_WRAP(srcM, patt2.l + i, patt2.t + j);
            if (a != b) return 0;
        }
    }
    return 1;
}

struct wfc__Pattern* wfc__gatherPatterns(int n, struct wfc__Mda_2cu32 srcM,
    int *cnt) {
    struct wfc__Pattern *patts = NULL;

    int pattCnt = 0;
    for (int px = 0; px < WFC__MDA_LEN(srcM); ++px) {
        struct wfc__Pattern patt = {0};
        wfc__indToCoords2d(srcM.dim[0], px, &patt.l, &patt.t);

        int seenBefore = 0;
        for (int px1 = 0; !seenBefore && px1 < px; ++px1) {
            struct wfc__Pattern patt1 = {0};
            wfc__indToCoords2d(srcM.dim[0], px1, &patt1.l, &patt1.t);

            if (wfc__patternsEq(n, srcM, patt, patt1)) seenBefore = 1;
        }

        if (!seenBefore) ++pattCnt;
    }

    patts = malloc((size_t)pattCnt * sizeof(*patts));
    pattCnt = 0;
    for (int px = 0; px < WFC__MDA_LEN(srcM); ++px) {
        struct wfc__Pattern patt = {0};
        wfc__indToCoords2d(srcM.dim[0], px, &patt.l, &patt.t);
        patt.freq = 1;

        int seenBefore = 0;
        for (int i = 0; !seenBefore && i < pattCnt; ++i) {
            if (wfc__patternsEq(n, srcM, patt, patts[i])) {
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
    const struct wfc__Pattern *patts,
    struct wfc__Mda_3u8 wave,
    struct wfc__Mda_2f *entropies) {
    for (int x = 0; x < entropies->dim[0]; ++x) {
        for (int y = 0; y < entropies->dim[1]; ++y) {
            int totalFreq = 0;
            int availPatts = 0;
            for (int z = 0; z < wave.dim[2]; ++z) {
                if (WFC__MDA_GET(wave, x, y, z)) {
                    totalFreq += patts[z].freq;
                    ++availPatts;
                }
            }

            float entropy = 0.0f;
            // check is here to ensure entropy of observed points becomes 0
            if (availPatts > 1) {
                for (int z = 0; z < wave.dim[2]; ++z) {
                    if (WFC__MDA_GET(wave, x, y, z)) {
                        float prob = (float)patts[z].freq / (float)totalFreq;
                        entropy -= prob * log2f(prob);
                    }
                }
            }

            WFC__MDA_GET(*entropies, x, y) = entropy;
        }
    }
}

void wfc__observeOne(
    int pattCnt, const struct wfc__Pattern *patts,
    struct wfc__Mda_2f entropies,
    int *obsX, int *obsY, struct wfc__Mda_3u8 *wave) {
    float smallest;
    int smallestCnt = 0;
    for (int i = 0; i < WFC__MDA_LEN(entropies); ++i) {
        // skip observed points
        if (entropies.a[i] == 0.0f) continue;

        if (smallestCnt > 0 && wfc__approxEq_f(entropies.a[i], smallest)) {
            ++smallestCnt;
        } else if (smallestCnt == 0 || entropies.a[i] < smallest) {
            smallest = entropies.a[i];
            smallestCnt = 1;
        }
    }

    int chosenX, chosenY;
    {
        int chosenPnt = 0;
        int chosenSmallestPnt = wfc__rand_i(smallestCnt);
        for (int i = 0; i < WFC__MDA_LEN(entropies); ++i) {
            if (wfc__approxEq_f(entropies.a[i], smallest)) {
                chosenPnt = i;
                if (chosenSmallestPnt == 0) break;
                --chosenSmallestPnt;
            }
        }
        wfc__indToCoords2d(entropies.dim[0], chosenPnt, &chosenX, &chosenY);
    }

    int chosenPatt = 0;
    {
        int totalFreq = 0;
        for (int i = 0; i < pattCnt; ++i) {
            if (WFC__MDA_GET(*wave, chosenX, chosenY, i)) {
                totalFreq += patts[i].freq;
            }
        }
        int chosenInst = wfc__rand_i(totalFreq);

        for (int i = 0; i < pattCnt; ++i) {
            if (WFC__MDA_GET(*wave, chosenX, chosenY, i)) {
                if (chosenInst < patts[i].freq) {
                    chosenPatt = i;
                    break;
                }
                chosenInst -= patts[i].freq;
            }
        }
    }

    *obsX = chosenX;
    *obsY = chosenY;
    for (int i = 0; i < pattCnt; ++i) {
        WFC__MDA_GET(*wave, chosenX, chosenY, i) = 0;
    }
    WFC__MDA_GET(*wave, chosenX, chosenY, chosenPatt) = 1;
}

int wfc__overlapMatches(
    int n, struct wfc__Mda_2cu32 srcM,
    int x1, int y1, int x2, int y2,
    struct wfc__Pattern patt1, struct wfc__Pattern patt2) {
    assert(abs(x1 - x2) < n);
    assert(abs(y1 - y2) < n);

    int overlapW = n - abs(x1 - x2);
    int overlapH = n - abs(y1 - y2);

    int overlapX = x1 < x2 ? x2 : x1;
    int overlapY = y1 < y2 ? y2 : y1;

    int overlapPX1 = overlapX - x1;
    int overlapPY1 = overlapY - y1;
    int overlapPX2 = overlapX - x2;
    int overlapPY2 = overlapY - y2;

    for (int i = 0; i < overlapW; ++i) {
        for (int j = 0; j < overlapH; ++j) {
            int px1 = overlapPX1 + i;
            int py1 = overlapPY1 + j;
            int px2 = overlapPX2 + i;
            int py2 = overlapPY2 + j;

            int mx1 = patt1.l + px1;
            int my1 = patt1.t + py1;
            int mx2 = patt2.l + px2;
            int my2 = patt2.t + py2;

            uint32_t a = WFC__MDA_GET_WRAP(srcM, mx1, my1);
            uint32_t b = WFC__MDA_GET_WRAP(srcM, mx2, my2);

            if (a != b) return 0;
        }
    }

    return 1;
}

int wfc__propagateSingle(
    int n, struct wfc__Mda_2cu32 srcM,
    int x, int y, int xN, int yN,
    int pattCnt, const struct wfc__Pattern *patts,
    struct wfc__Mda_3u8 *wave) {
    int changed = 0;

    for (int p = 0; p < pattCnt; ++p) {
        if (WFC__MDA_GET_WRAP(*wave, x, y, p)) {
            int mayKeep = 0;
            for (int pN = 0; pN < pattCnt; ++pN) {
                if (WFC__MDA_GET_WRAP(*wave, xN, yN, pN)) {
                    if (wfc__overlapMatches(n, srcM,
                            x, y, xN, yN, patts[p], patts[pN])) {
                        mayKeep = 1;
                        break;
                    }
                }
            }

            if (!mayKeep) {
                WFC__MDA_GET_WRAP(*wave, x, y, p) = 0;
                changed = 1;
            }
        }
    }

    return changed;
}

void wfc__propagate(
    int n, struct wfc__Mda_2cu32 srcM,
    int pattCnt, const struct wfc__Pattern *patts,
    int seedX, int seedY, struct wfc__Mda_2u8 *ripple,
    struct wfc__Mda_3u8 *wave) {
    memset(ripple->a, 0, WFC__MDA_SIZE(*ripple));
    WFC__MDA_GET(*ripple, seedX, seedY) = 1;

    uint8_t oddEven = 0;

    while (1) {
        uint8_t oddEvenMask = (uint8_t)(1 << oddEven);
        uint8_t oddEvenMaskNext = (uint8_t)(1 << (1 - oddEven));

        int done = 1;
        for (int x = 0; x < ripple->dim[0]; ++x) {
            for (int y = 0; y < ripple->dim[1]; ++y) {
                if (WFC__MDA_GET(*ripple, x, y) & oddEvenMask) done = 0;
                WFC__MDA_GET(*ripple, x, y) &= ~oddEvenMaskNext;
            }
        }

        if (done) break;

        for (int xN = 0; xN < wave->dim[0]; ++xN) {
            for (int yN = 0; yN < wave->dim[1]; ++yN) {
                if (!(WFC__MDA_GET(*ripple, xN, yN) & oddEvenMask)) continue;

                for (int dx = -(n - 1); dx <= n - 1; ++dx) {
                    for (int dy = -(n - 1); dy <= n - 1; ++dy) {
                        int x = xN + dx;
                        int y = yN + dy;

                        if (wfc__propagateSingle(n, srcM,
                                x, y, xN, yN, pattCnt, patts, wave)) {
                            WFC__MDA_GET_WRAP(*ripple, x, y) |= oddEvenMask;
                            WFC__MDA_GET_WRAP(*ripple, x, y) |= oddEvenMaskNext;
                        }
                    }
                }
            }
        }

        oddEven = 1 - oddEven;
    }
}

int wfc_generate(
    int n,
    int srcW, int srcH, const uint32_t *src,
    int dstW, int dstH, uint32_t *dst) {
    assert(n > 0);
    assert(srcW > 0);
    assert(srcH > 0);
    assert(src != NULL);
    assert(dstW > 0);
    assert(dstH > 0);
    assert(dst != NULL);

    int ret = 0;

    struct wfc__Pattern *patts = NULL;
    struct wfc__Mda_3u8 wave = {0};
    struct wfc__Mda_2f entropies = {0};
    struct wfc__Mda_2u8 ripple = {0};

    struct wfc__Mda_2cu32 srcM = {src, {srcW, srcH}};
    struct wfc__Mda_2u32 dstM = {dst, {dstW, dstH}};

    int pattCnt;
    patts = wfc__gatherPatterns(n, srcM, &pattCnt);

    wave.dim[0] = dstW;
    wave.dim[1] = dstH;
    wave.dim[2] = pattCnt;
    wave.a = malloc(WFC__MDA_SIZE(wave));
    for (int i = 0; i < WFC__MDA_LEN(wave); ++i) wave.a[i] = 1;

    entropies.dim[0] = dstW;
    entropies.dim[1] = dstH;
    entropies.a = malloc(WFC__MDA_SIZE(entropies));

    ripple.dim[0] = dstW;
    ripple.dim[1] = dstH;
    ripple.a = malloc(WFC__MDA_SIZE(ripple));

    while (1) {
        int minPatts = pattCnt, maxPatts = 0;
        for (int x = 0; x < wave.dim[0]; ++x) {
            for (int y = 0; y < wave.dim[1]; ++y) {
                int patts = 0;
                for (int z = 0; z < wave.dim[2]; ++z) {
                    if (WFC__MDA_GET(wave, x, y, z)) ++patts;
                }

                if (patts < minPatts) minPatts = patts;
                if (patts > maxPatts) maxPatts = patts;
            }
        }

        if (minPatts == 0) {
            // contradiction reached
            ret = 1;
            goto cleanup;
        } else if (maxPatts == 1) {
            // WFC completed
            break;
        }

        wfc__calcEntropies(patts, wave, &entropies);

        int obsX, obsY;
        wfc__observeOne(pattCnt, patts, entropies, &obsX, &obsY, &wave);

        wfc__propagate(n, srcM, pattCnt, patts, obsX, obsY, &ripple, &wave);
    }

    for (int x = 0; x < dstW; ++x) {
        for (int y = 0; y < dstH; ++y) {
            int patt = 0;
            for (int z = 0; z < pattCnt; ++z) {
                if (WFC__MDA_GET(wave, x, y, z)) {
                    patt = z;
                    break;
                }
            }

            int mx = patts[patt].l;
            int my = patts[patt].t;
            uint32_t col = WFC__MDA_GET(srcM, mx, my);

            WFC__MDA_GET(dstM, x, y) = col;
        }
    }

cleanup:
    free(ripple.a);
    free(entropies.a);
    free(wave.a);
    free(patts);

    return ret;
}
