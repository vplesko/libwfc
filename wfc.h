#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// @TODO verify correctness
// @TODO allow users to supply their own assert
// @TODO allow users to supply their own malloc et al.
// @TODO allow users to supply their own rand

// [0, 1)
float wfc__rand(void) {
    return 1.0f * rand() / ((float)RAND_MAX + 1);
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
    ((mat).m[wfc__mat2dXyToInd((mat).w, x, y)])

#define WFC__MAT2DGETWRAP(mat, x, y) \
    ((mat).m[wfc__mat2dXyToInd( \
        (mat).w, \
        wfc__indWrap(x, (mat).w), \
        wfc__indWrap(y, (mat).h))\
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

WFC__DEF_MAT2D(const uint32_t, cu32);
WFC__DEF_MAT2D(float, f);
WFC__DEF_MAT3D(uint8_t, u8);

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

            float entropy = 0;
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
    struct wfc__Mat3d_u8 *wave) {
    float smallest = WFC__MAT2DGET(entropies, 0, 0);
    int smallestCnt = 1;
    for (int i = 1; i < entropies.w * entropies.h; ++i) {
        // skip observed points
        if (entropies.m[i] == 0) continue;

        if (wfc__approxEq_f(entropies.m[i], smallest)) {
            ++smallestCnt;
        } else if (entropies.m[i] < smallest) {
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

void wfc__propagateSingle(
    int n, struct wfc__Mat2d_cu32 srcM,
    int x, int y, int xN, int yN,
    int pattCnt, const struct wfc__Pattern *patts,
    struct wfc__Mat3d_u8 *wave) {
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
            }
        }
    }
}

void wfc__propagate(
    int n, struct wfc__Mat2d_cu32 srcM,
    int pattCnt, const struct wfc__Pattern *patts,
    struct wfc__Mat3d_u8 *wave) {
    // @TODO repeat until no changes are made
    for (int x = 0; x < wave->w; ++x) {
        for (int y = 0; y < wave->h; ++y) {
            for (int dx = -(n - 1); dx <= n - 1; ++dx) {
                for (int dy = -(n - 1); dy <= n - 1; ++dy) {
                    int xN = x + dx;
                    int yN = y + dy;

                    wfc__propagateSingle(n, srcM,
                        x, y, xN, yN, pattCnt, patts, wave);
                }
            }
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

    struct wfc__Mat2d_cu32 srcM = {src, srcW, srcH};

    int pattCnt;
    patts = wfc__gatherPatterns(n, srcM, &pattCnt);

    wave.w = dstW;
    wave.h = dstH;
    wave.d = pattCnt;
    wave.m = malloc(wave.w * wave.h * wave.d * sizeof(*wave.m));
    for (int i = 0; i < wave.w * wave.h * wave.d; ++i) wave.m[i] = 1;

    entropies.w = dstW;
    entropies.h = dstH;
    entropies.m = malloc(entropies.w * entropies.h * sizeof(*entropies.m));

    // @TODO repeat until all have single pattern or contradiction reached
    wfc__calcEntropies(patts, wave, &entropies);
    wfc__observeOne(pattCnt, patts, entropies, &wave);
    // @TODO detect contradictions
    wfc__propagate(n, srcM, pattCnt, patts, &wave);

    // dummy impl
    int mostCommonPatt = 0;
    for (int i = 0; i < pattCnt; ++i) {
        if (patts[i].freq > mostCommonPatt) mostCommonPatt = patts[i].freq;
    }
    wfc__calcEntropies(patts, wave, &entropies);
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
    /*for (int x = 0; x < srcW; ++x) {
        for (int y = 0; y < srcH; ++y) {
            struct wfc__Pattern patt = {x, y, 0};

            for (int i = 0; i < pattCnt; ++i) {
                if (wfc__patternsEq(n, srcM, patt, patts[i])) {
                    int red = patts[i].freq * 255 / mostCommonPatt;
                    dst[wfc__mat2dXyToInd(dstW, x, y)] = 0xff000000 + red;
                }
            }
        }
    }*/

//cleanup:
    free(entropies.m);
    free(wave.m);
    free(patts);

    return 0;
}
