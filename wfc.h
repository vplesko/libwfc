#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// @TODO allow users to supply their own assert
// @TODO allow users to supply their own malloc et al.
// @TODO allow users to supply their own rand
// @TODO add restrict where appropriate

// [0, 1)
float wfc__rand(void) {
    return 1.0f * rand() / ((float)RAND_MAX + 1.0f);
}

// [0, n)
int wfc__rand_i(int n) {
    return wfc__rand() * n;
}

int wfc__approxEq_f(float a, float b) {
    const float absDiff = 0.001f;
    const float relDiff = FLT_EPSILON;

    if (fabsf(a - b) < absDiff) return 1;

    if (fabsf(a) < fabsf(b)) return fabsf((a - b) / b) < relDiff;
    return fabsf((a - b) / a) < relDiff;
}

int wfc__indWrap(int ind, int sz) {
    if (ind >= 0) return ind % sz;
    return sz + ind % sz;
}

// @TODO add macros with range as arg
#define WFC__MAT2DDEF(type, abbrv) \
    struct wfc__Mat2d_##abbrv { \
        type *m; \
        int w, h; \
    }

#define WFC__MAT2DSIZE(mat) ((mat).w * (mat).h * sizeof(*(mat).m))

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
    ((mat).m[wfc__mat2dXyToInd((mat).w, x, y)])

#define WFC__MAT2DGETWRAP(mat, x, y) \
    ((mat).m[wfc__mat2dXyToInd( \
        (mat).w, \
        wfc__indWrap(x, (mat).w), \
        wfc__indWrap(y, (mat).h))\
    ])

#define WFC__MAT3DDEF(type, abbrv) \
    struct wfc__Mat3d_##abbrv { \
        type *m; \
        int w, h, d; \
    }

#define WFC__MAT3DSIZE(mat) ((mat).w * (mat).h * (mat).d * sizeof(*(mat).m))

int wfc__mat3dXyzToInd(int w, int h, int x, int y, int z) {
    return z * w * h + y * w + x;
}

void wfc__mat3dIndToXyz(int w, int h, int ind, int *x, int *y, int *z) {
    int zz = ind / (w * h);
    ind -= zz * (w * h);
    int yy = ind / w;
    ind -= yy * w;
    int xx = ind;

    *x = xx;
    *y = yy;
    *z = zz;
}

#define WFC__MAT3DGET(mat, x, y, z) \
    ((mat).m[wfc__mat3dXyzToInd((mat).w, (mat).h, x, y, z)])

#define WFC__MAT3DGETWRAP(mat, x, y, z) \
    ((mat).m[wfc__mat3dXyzToInd( \
        (mat).w, \
        (mat).h, \
        wfc__indWrap(x, (mat).w), \
        wfc__indWrap(y, (mat).h), \
        wfc__indWrap(z, (mat).d))\
    ])

struct wfc__Pattern {
    int l, t;
    int freq;
};

WFC__MAT2DDEF(const uint32_t, cu32);
WFC__MAT2DDEF(uint8_t, u8);
WFC__MAT2DDEF(float, f);
WFC__MAT3DDEF(uint8_t, u8);

int wfc__patternsEq(int n, struct wfc__Mat2d_cu32 srcM,
    struct wfc__Pattern patt1, struct wfc__Pattern patt2) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            uint32_t a = WFC__MAT2DGETWRAP(srcM, patt1.l + i, patt1.t + j);
            uint32_t b = WFC__MAT2DGETWRAP(srcM, patt2.l + i, patt2.t + j);
            if (a != b) return 0;
        }
    }
    return 1;
}

struct wfc__Pattern* wfc__gatherPatterns(int n, struct wfc__Mat2d_cu32 srcM,
    int *cnt) {
    struct wfc__Pattern *patts = NULL;

    int pattCnt = 0;
    for (int px = 0; px < srcM.w * srcM.h; ++px) {
        struct wfc__Pattern patt = {0};
        wfc__mat2dIndToXy(srcM.w, px, &patt.l, &patt.t);

        int seenBefore = 0;
        for (int px1 = 0; !seenBefore && px1 < px; ++px1) {
            struct wfc__Pattern patt1 = {0};
            wfc__mat2dIndToXy(srcM.w, px1, &patt1.l, &patt1.t);

            if (wfc__patternsEq(n, srcM, patt, patt1)) seenBefore = 1;
        }

        if (!seenBefore) ++pattCnt;
    }

    patts = malloc(pattCnt * sizeof(*patts));
    pattCnt = 0;
    for (int px = 0; px < srcM.w * srcM.h; ++px) {
        struct wfc__Pattern patt = {0};
        wfc__mat2dIndToXy(srcM.w, px, &patt.l, &patt.t);
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
    struct wfc__Mat3d_u8 wave,
    struct wfc__Mat2d_f *entropies) {
    for (int x = 0; x < entropies->w; ++x) {
        for (int y = 0; y < entropies->h; ++y) {
            int totalFreq = 0;
            int availPatts = 0;
            for (int z = 0; z < wave.d; ++z) {
                if (WFC__MAT3DGET(wave, x, y, z)) {
                    totalFreq += patts[z].freq;
                    ++availPatts;
                }
            }

            float entropy = 0.0f;
            // check is here to ensure entropy of observed points becomes 0
            if (availPatts > 1) {
                for (int z = 0; z < wave.d; ++z) {
                    if (WFC__MAT3DGET(wave, x, y, z)) {
                        float prob = 1.0f * patts[z].freq / totalFreq;
                        entropy -= prob * log2f(prob);
                    }
                }
            }

            WFC__MAT2DGET(*entropies, x, y) = entropy;
        }
    }
}

void wfc__observeOne(
    int pattCnt, const struct wfc__Pattern *patts,
    struct wfc__Mat2d_f entropies,
    int *obsX, int *obsY, struct wfc__Mat3d_u8 *wave) {
    float smallest;
    int smallestCnt = 0;
    for (int i = 0; i < entropies.w * entropies.h; ++i) {
        // skip observed points
        if (entropies.m[i] == 0.0f) continue;

        if (smallestCnt > 0 && wfc__approxEq_f(entropies.m[i], smallest)) {
            ++smallestCnt;
        } else if (smallestCnt == 0 || entropies.m[i] < smallest) {
            smallest = entropies.m[i];
            smallestCnt = 1;
        }
    }

    int chosenX, chosenY;
    {
        int chosenPnt = 0;
        int chosenSmallestPnt = wfc__rand_i(smallestCnt);
        for (int i = 0; i < entropies.w * entropies.h; ++i) {
            if (wfc__approxEq_f(entropies.m[i], smallest)) {
                chosenPnt = i;
                if (chosenSmallestPnt == 0) break;
                --chosenSmallestPnt;
            }
        }
        wfc__mat2dIndToXy(entropies.w, chosenPnt, &chosenX, &chosenY);
    }

    int chosenPatt = 0;
    {
        int totalFreq = 0;
        for (int i = 0; i < pattCnt; ++i) {
            if (WFC__MAT3DGET(*wave, chosenX, chosenY, i)) {
                totalFreq += patts[i].freq;
            }
        }
        int chosenInst = wfc__rand_i(totalFreq);

        for (int i = 0; i < pattCnt; ++i) {
            if (WFC__MAT3DGET(*wave, chosenX, chosenY, i)) {
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
        WFC__MAT3DGET(*wave, chosenX, chosenY, i) = 0;
    }
    WFC__MAT3DGET(*wave, chosenX, chosenY, chosenPatt) = 1;
}

int wfc__overlapMatches(
    int n, struct wfc__Mat2d_cu32 srcM,
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

            uint32_t a = WFC__MAT2DGETWRAP(srcM, mx1, my1);
            uint32_t b = WFC__MAT2DGETWRAP(srcM, mx2, my2);

            if (a != b) return 0;
        }
    }

    return 1;
}

int wfc__propagateSingle(
    int n, struct wfc__Mat2d_cu32 srcM,
    int x, int y, int xN, int yN,
    int pattCnt, const struct wfc__Pattern *patts,
    struct wfc__Mat3d_u8 *wave) {
    int changed = 0;

    for (int p = 0; p < pattCnt; ++p) {
        if (WFC__MAT3DGETWRAP(*wave, x, y, p)) {
            int mayKeep = 0;
            for (int pN = 0; pN < pattCnt; ++pN) {
                if (WFC__MAT3DGETWRAP(*wave, xN, yN, pN)) {
                    if (wfc__overlapMatches(n, srcM,
                            x, y, xN, yN, patts[p], patts[pN])) {
                        mayKeep = 1;
                        break;
                    }
                }
            }

            if (!mayKeep) {
                WFC__MAT3DGETWRAP(*wave, x, y, p) = 0;
                changed = 1;
            }
        }
    }

    return changed;
}

void wfc__propagate(
    int n, struct wfc__Mat2d_cu32 srcM,
    int pattCnt, const struct wfc__Pattern *patts,
    int seedX, int seedY, struct wfc__Mat2d_u8 *ripple,
    struct wfc__Mat3d_u8 *wave) {
    memset(ripple->m, 0, WFC__MAT2DSIZE(*ripple));
    WFC__MAT2DGET(*ripple, seedX, seedY) = 1;

    uint8_t oddEven = 0;

    while (1) {
        uint8_t oddEvenMask = 1 << oddEven;
        uint8_t oddEvenMaskNext = 1 << (1 - oddEven);

        int done = 1;
        for (int x = 0; x < ripple->w; ++x) {
            for (int y = 0; y < ripple->h; ++y) {
                if (WFC__MAT2DGET(*ripple, x, y) & oddEvenMask) done = 0;
                WFC__MAT2DGET(*ripple, x, y) &= ~oddEvenMaskNext;
            }
        }

        if (done) break;

        for (int xN = 0; xN < wave->w; ++xN) {
            for (int yN = 0; yN < wave->h; ++yN) {
                if (!(WFC__MAT2DGET(*ripple, xN, yN) & oddEvenMask)) continue;

                for (int dx = -(n - 1); dx <= n - 1; ++dx) {
                    for (int dy = -(n - 1); dy <= n - 1; ++dy) {
                        int x = xN + dx;
                        int y = yN + dy;

                        if (wfc__propagateSingle(n, srcM,
                                x, y, xN, yN, pattCnt, patts, wave)) {
                            WFC__MAT2DGETWRAP(*ripple, x, y) |= oddEvenMask;
                            WFC__MAT2DGETWRAP(*ripple, x, y) |= oddEvenMaskNext;
                        }
                    }
                }
            }
        }

        oddEven = 1 - oddEven;
    }
}

// @TODO verify args
int wfc_generate(
    int n,
    int srcW, int srcH, const uint32_t *src,
    int dstW, int dstH, uint32_t *dst) {
    int ret = 0;

    struct wfc__Pattern *patts = NULL;
    struct wfc__Mat3d_u8 wave = {0};
    struct wfc__Mat2d_f entropies = {0};
    struct wfc__Mat2d_u8 ripple = {0};

    struct wfc__Mat2d_cu32 srcM = {src, srcW, srcH};

    int pattCnt;
    patts = wfc__gatherPatterns(n, srcM, &pattCnt);

    wave.w = dstW;
    wave.h = dstH;
    wave.d = pattCnt;
    wave.m = malloc(WFC__MAT3DSIZE(wave));
    for (int i = 0; i < wave.w * wave.h * wave.d; ++i) wave.m[i] = 1;

    entropies.w = dstW;
    entropies.h = dstH;
    entropies.m = malloc(WFC__MAT2DSIZE(entropies));

    ripple.w = dstW;
    ripple.h = dstH;
    ripple.m = malloc(WFC__MAT2DSIZE(ripple));

    while (1) {
        int minPatts = pattCnt, maxPatts = 0;
        for (int x = 0; x < wave.w; ++x) {
            for (int y = 0; y < wave.h; ++y) {
                int patts = 0;
                for (int z = 0; z < wave.d; ++z) {
                    if (WFC__MAT3DGET(wave, x, y, z)) ++patts;
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
                if (WFC__MAT3DGET(wave, x, y, z)) {
                    patt = z;
                    break;
                }
            }

            int mx = patts[patt].l;
            int my = patts[patt].t;
            uint32_t col = src[wfc__mat2dXyToInd(srcW, mx, my)];

            dst[wfc__mat2dXyToInd(dstW, x, y)] = col;
        }
    }

cleanup:
    free(ripple.m);
    free(entropies.m);
    free(wave.m);
    free(patts);

    return ret;
}
