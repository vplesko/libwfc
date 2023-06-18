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

// basic utility

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

int wfc__indWrap(int ind, int sz) {
    if (ind >= 0) return ind % sz;
    return sz + ind % sz;
}

// @TODO same for all MDAs
#define WFC__A3D_DEF(type, abbrv) \
    struct wfc__A3d_##abbrv { \
        type *a; \
        int d0, d1, d2; \
    }

#define WFC__A3D_LEN(arr) ((arr).d0 * (arr).d1 * (arr).d2)

#define WFC__A3D_SIZE(arr) ((size_t)WFC__A3D_LEN(arr) * sizeof(*(arr).a))

#define WFC__A3D_GET(arr, c0, c1, c2) \
    ((arr).a[ \
        (c2) * (arr).d0 * (arr).d1 + \
        (c1) * (arr).d0 + \
        (c0)])

#define WFC__A3D_GET_WRAP(arr, c0, c1, c2) \
    ((arr).a[ \
        wfc__indWrap(c2, (arr).d2) * (arr).d0 * (arr).d1 + \
        wfc__indWrap(c1, (arr).d1) * (arr).d0 + \
        wfc__indWrap(c0, (arr).d0)])

#define WFC__A4D_DEF(type, abbrv) \
    struct wfc__A4d_##abbrv { \
        type *a; \
        int d0, d1, d2, d3; \
    }

#define WFC__A4D_LEN(arr) ((arr).d0 * (arr).d1 * (arr).d2 * (arr).d3)

#define WFC__A4D_SIZE(arr) ((size_t)WFC__A4D_LEN(arr) * sizeof(*(arr).a))

#define WFC__A4D_GET(arr, c0, c1, c2, c3) \
    ((arr).a[ \
        (c3) * (arr).d0 * (arr).d1 * (arr).d2 + \
        (c2) * (arr).d0 * (arr).d1 + \
        (c1) * (arr).d0 + \
        (c0)])

#define WFC__A4D_GET_WRAP(arr, c0, c1, c2, c3) \
    ((arr).a[ \
        wfc__indWrap(c3, (arr).d3) * (arr).d0 * (arr).d1 * (arr).d2 + \
        wfc__indWrap(c2, (arr).d2) * (arr).d0 * (arr).d1 + \
        wfc__indWrap(c1, (arr).d1) * (arr).d0 + \
        wfc__indWrap(c0, (arr).d0)])

struct wfc__Size2d { int dim[2]; };

int wfc__len2d(struct wfc__Size2d sz) {
    return sz.dim[0] * sz.dim[1];
}

size_t wfc__size2d(struct wfc__Size2d sz, size_t membSz) {
    return (size_t)wfc__len2d(sz) * membSz;
}

int wfc__coordsToInd2d(struct wfc__Size2d sz, int c0, int c1) {
    return c1 * sz.dim[0] + c0;
}

int wfc__coordsToInd2dWrap(struct wfc__Size2d sz, int c0, int c1) {
    return wfc__coordsToInd2d(
        sz,
        wfc__indWrap(c0, sz.dim[0]),
        wfc__indWrap(c1, sz.dim[1]));
}

void wfc__indToCoords2d(struct wfc__Size2d sz, int ind, int *c0, int *c1) {
    int c1_ = ind / sz.dim[0];
    ind -= c1_ * sz.dim[0];
    int c0_ = ind;

    *c0 = c0_;
    *c1 = c1_;
}

// wfc code

struct wfc__Pattern {
    int l, t;
    int freq;
};

WFC__A3D_DEF(uint8_t, u8);
WFC__A4D_DEF(uint8_t, u8);

int wfc__patternsEq(
    int n,
    struct wfc__Size2d srcSz, const uint32_t *src,
    struct wfc__Pattern patt1, struct wfc__Pattern patt2) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            uint32_t a = src[
                wfc__coordsToInd2dWrap(srcSz, patt1.l + i, patt1.t + j)];
            uint32_t b = src[
                wfc__coordsToInd2dWrap(srcSz, patt2.l + i, patt2.t + j)];;
            if (a != b) return 0;
        }
    }
    return 1;
}

struct wfc__Pattern* wfc__gatherPatterns(
    int n,
    struct wfc__Size2d srcSz, const uint32_t *src,
    int *cnt) {
    struct wfc__Pattern *patts = NULL;

    int pattCnt = 0;
    for (int px = 0; px < wfc__len2d(srcSz); ++px) {
        struct wfc__Pattern patt = {0};
        wfc__indToCoords2d(srcSz, px, &patt.l, &patt.t);

        int seenBefore = 0;
        for (int px1 = 0; !seenBefore && px1 < px; ++px1) {
            struct wfc__Pattern patt1 = {0};
            wfc__indToCoords2d(srcSz, px1, &patt1.l, &patt1.t);

            if (wfc__patternsEq(n, srcSz, src, patt, patt1)) seenBefore = 1;
        }

        if (!seenBefore) ++pattCnt;
    }

    patts = malloc((size_t)pattCnt * sizeof(*patts));
    pattCnt = 0;
    for (int px = 0; px < wfc__len2d(srcSz); ++px) {
        struct wfc__Pattern patt = {0};
        wfc__indToCoords2d(srcSz, px, &patt.l, &patt.t);
        patt.freq = 1;

        int seenBefore = 0;
        for (int i = 0; !seenBefore && i < pattCnt; ++i) {
            if (wfc__patternsEq(n, srcSz, src, patt, patts[i])) {
                ++patts[i].freq;
                seenBefore = 1;
            }
        }

        if (!seenBefore) patts[pattCnt++] = patt;
    }

    *cnt = pattCnt;
    return patts;
}

int wfc__overlapMatches(
    int n,
    struct wfc__Size2d srcSz, const uint32_t *src,
    struct wfc__Pattern patt1, struct wfc__Pattern patt2,
    int dx, int dy) {
    assert(abs(dx) < n);
    assert(abs(dy) < n);

    int overlapW = n - abs(dx);
    int overlapH = n - abs(dy);

    int overlapPX1 = dx > 0 ? dx : 0;
    int overlapPY1 = dy > 0 ? dy : 0;
    int overlapPX2 = dx > 0 ? 0 : abs(dx);
    int overlapPY2 = dy > 0 ? 0 : abs(dy);

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

            uint32_t a = src[wfc__coordsToInd2dWrap(srcSz, mx1, my1)];
            uint32_t b = src[wfc__coordsToInd2dWrap(srcSz, mx2, my2)];

            if (a != b) return 0;
        }
    }

    return 1;
}

struct wfc__A4d_u8 wfc__calcOverlaps(
    int n,
    struct wfc__Size2d srcSz, const uint32_t *src,
    int pattCnt, const struct wfc__Pattern *patts) {
    struct wfc__A4d_u8 overlaps;
    overlaps.d0 = pattCnt;
    overlaps.d1 = pattCnt;
    overlaps.d2 = 2 * n - 1;
    overlaps.d3 = 2 * n - 1;
    overlaps.a = malloc(WFC__A4D_SIZE(overlaps));

    for (int i = 0; i < pattCnt; ++i) {
        for (int j = 0; j < pattCnt; ++j) {
            for (int dx = -(n - 1); dx <= n - 1; ++dx) {
                for (int dy = -(n - 1); dy <= n - 1; ++dy) {
                    int overlap = wfc__overlapMatches(n, srcSz, src,
                        patts[i], patts[j], dx, dy);
                    WFC__A4D_GET(overlaps, i, j, dx + n - 1, dy + n - 1) =
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
    float *entropies) {
    struct wfc__Size2d dstSz = {{wave.d0, wave.d1}};

    for (int x = 0; x < wave.d0; ++x) {
        for (int y = 0; y < wave.d1; ++y) {
            int totalFreq = 0;
            int availPatts = 0;
            for (int z = 0; z < wave.d2; ++z) {
                if (WFC__A3D_GET(wave, x, y, z)) {
                    totalFreq += patts[z].freq;
                    ++availPatts;
                }
            }

            float entropy = 0.0f;
            // check is here to ensure entropy of observed points becomes 0
            if (availPatts > 1) {
                for (int z = 0; z < wave.d2; ++z) {
                    if (WFC__A3D_GET(wave, x, y, z)) {
                        float prob = (float)patts[z].freq / (float)totalFreq;
                        entropy -= prob * log2f(prob);
                    }
                }
            }

            entropies[wfc__coordsToInd2d(dstSz, x, y)] = entropy;
        }
    }
}

void wfc__observeOne(
    int pattCnt, const struct wfc__Pattern *patts,
    const float *entropies,
    int *obsX, int *obsY,
    struct wfc__A3d_u8 wave) {
    struct wfc__Size2d dstSz = {{wave.d0, wave.d1}};

    float smallest;
    int smallestCnt = 0;
    for (int i = 0; i < wfc__len2d(dstSz); ++i) {
        // skip observed points
        if (entropies[i] == 0.0f) continue;

        if (smallestCnt > 0 && wfc__approxEq_f(entropies[i], smallest)) {
            ++smallestCnt;
        } else if (smallestCnt == 0 || entropies[i] < smallest) {
            smallest = entropies[i];
            smallestCnt = 1;
        }
    }

    int chosenX, chosenY;
    {
        int chosenPnt = 0;
        int chosenSmallestPnt = wfc__rand_i(smallestCnt);
        for (int i = 0; i < wfc__len2d(dstSz); ++i) {
            if (wfc__approxEq_f(entropies[i], smallest)) {
                chosenPnt = i;
                if (chosenSmallestPnt == 0) break;
                --chosenSmallestPnt;
            }
        }
        wfc__indToCoords2d(dstSz, chosenPnt, &chosenX, &chosenY);
    }

    int chosenPatt = 0;
    {
        int totalFreq = 0;
        for (int i = 0; i < pattCnt; ++i) {
            if (WFC__A3D_GET(wave, chosenX, chosenY, i)) {
                totalFreq += patts[i].freq;
            }
        }
        int chosenInst = wfc__rand_i(totalFreq);

        for (int i = 0; i < pattCnt; ++i) {
            if (WFC__A3D_GET(wave, chosenX, chosenY, i)) {
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
        WFC__A3D_GET(wave, chosenX, chosenY, i) = 0;
    }
    WFC__A3D_GET(wave, chosenX, chosenY, chosenPatt) = 1;
}

int wfc__propagateSingle(
    int n, int x, int y, int xN, int yN,
    struct wfc__A4d_u8 overlaps,
    struct wfc__A3d_u8 wave) {
    int pattCnt = overlaps.d0;

    int changed = 0;

    for (int p = 0; p < pattCnt; ++p) {
        if (WFC__A3D_GET_WRAP(wave, x, y, p)) {
            int mayKeep = 0;
            for (int pN = 0; pN < pattCnt; ++pN) {
                if (WFC__A3D_GET_WRAP(wave, xN, yN, pN)) {
                    if (WFC__A4D_GET(overlaps,
                            p, pN, xN - x + n - 1, yN - y + n - 1)) {
                        mayKeep = 1;
                        break;
                    }
                }
            }

            if (!mayKeep) {
                WFC__A3D_GET_WRAP(wave, x, y, p) = 0;
                changed = 1;
            }
        }
    }

    return changed;
}

void wfc__propagate(
    int n, int seedX, int seedY,
    struct wfc__A4d_u8 overlaps,
    uint8_t *ripple,
    struct wfc__A3d_u8 wave) {
    struct wfc__Size2d dstSz = {{wave.d0, wave.d1}};

    memset(ripple, 0, wfc__size2d(dstSz, sizeof(*ripple)));
    ripple[wfc__coordsToInd2d(dstSz, seedX, seedY)] = 1;

    int done = 0;
    while (!done) {
        done = 1;

        for (int xN = 0; xN < wave.d0; ++xN) {
            for (int yN = 0; yN < wave.d1; ++yN) {
                if (!ripple[wfc__coordsToInd2d(dstSz, xN, yN)]) continue;

                for (int dx = -(n - 1); dx <= n - 1; ++dx) {
                    for (int dy = -(n - 1); dy <= n - 1; ++dy) {
                        int x = xN + dx;
                        int y = yN + dy;

                        if (wfc__propagateSingle(n, x, y, xN, yN,
                                overlaps, wave)) {
                            ripple[wfc__coordsToInd2dWrap(dstSz, x, y)] = 1;
                            done = 0;
                        }
                    }
                }

                ripple[wfc__coordsToInd2d(dstSz, xN, yN)] = 0;
            }
        }
    }
}

int wfc_generate(
    int n,
    int srcW, int srcH, const uint32_t *src,
    int dstW, int dstH, uint32_t *dst) {
    assert(n > 0);
    assert(n <= srcW);
    assert(n <= srcH);
    assert(n <= dstW);
    assert(n <= dstH);
    assert(srcW > 0);
    assert(srcH > 0);
    assert(src != NULL);
    assert(dstW > 0);
    assert(dstH > 0);
    assert(dst != NULL);

    int ret = 0;

    struct wfc__Pattern *patts = NULL;
    struct wfc__A3d_u8 wave = {0};
    float *entropies = NULL;
    uint8_t *ripple = NULL;
    struct wfc__A4d_u8 overlaps = {0};

    struct wfc__Size2d srcSz = {{srcW, srcH}};
    struct wfc__Size2d dstSz = {{dstW, dstH}};

    int pattCnt;
    patts = wfc__gatherPatterns(n, srcSz, src, &pattCnt);

    wave.d0 = dstW;
    wave.d1 = dstH;
    wave.d2 = pattCnt;
    wave.a = malloc(WFC__A3D_SIZE(wave));
    for (int i = 0; i < WFC__A3D_LEN(wave); ++i) wave.a[i] = 1;

    entropies = malloc(wfc__size2d(dstSz, sizeof(*entropies)));

    ripple = malloc(wfc__size2d(dstSz, sizeof(*ripple)));

    overlaps = wfc__calcOverlaps(n, srcSz, src, pattCnt, patts);

    while (1) {
        int minPatts = pattCnt, maxPatts = 0;
        for (int x = 0; x < wave.d0; ++x) {
            for (int y = 0; y < wave.d1; ++y) {
                int patts = 0;
                for (int z = 0; z < wave.d2; ++z) {
                    if (WFC__A3D_GET(wave, x, y, z)) ++patts;
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

        wfc__calcEntropies(patts, wave, entropies);

        int obsX, obsY;
        wfc__observeOne(pattCnt, patts, entropies, &obsX, &obsY, wave);

        wfc__propagate(n, obsX, obsY, overlaps, ripple, wave);
    }

    for (int x = 0; x < dstW; ++x) {
        for (int y = 0; y < dstH; ++y) {
            int patt = 0;
            for (int z = 0; z < pattCnt; ++z) {
                if (WFC__A3D_GET(wave, x, y, z)) {
                    patt = z;
                    break;
                }
            }

            int mx = patts[patt].l;
            int my = patts[patt].t;
            uint32_t col = src[wfc__coordsToInd2d(srcSz, mx, my)];

            dst[wfc__coordsToInd2d(dstSz, x, y)] = col;
        }
    }

cleanup:
    free(overlaps.a);
    free(ripple);
    free(entropies);
    free(wave.a);
    free(patts);

    return ret;
}
