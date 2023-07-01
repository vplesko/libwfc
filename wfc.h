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

// @TODO rename to d02, d12; same for others
#define WFC__A2D_DEF(type, abbrv) \
    struct wfc__A2d_##abbrv { \
        int d20, d21; \
        type *a; \
    }

#define WFC__A2D_LEN(arr) ((arr).d20 * (arr).d21)

#define WFC__A2D_SIZE(arr) ((size_t)WFC__A2D_LEN(arr) * sizeof(*(arr).a))

#define WFC__A2D_GET(arr, c0, c1) \
    ((arr).a[ \
        (c0) * (arr).d21 + \
        (c1)])

#define WFC__A2D_GET_WRAP(arr, c0, c1) \
    ((arr).a[ \
        wfc__indWrap(c0, (arr).d20) * (arr).d21 + \
        wfc__indWrap(c1, (arr).d21)])

#define WFC__A3D_DEF(type, abbrv) \
    struct wfc__A3d_##abbrv { \
        int d30, d31, d32; \
        type *a; \
    }

#define WFC__A3D_LEN(arr) ((arr).d30 * (arr).d31 * (arr).d32)

#define WFC__A3D_SIZE(arr) ((size_t)WFC__A3D_LEN(arr) * sizeof(*(arr).a))

#define WFC__A3D_GET(arr, c0, c1, c2) \
    ((arr).a[ \
        (c0) * (arr).d31 * (arr).d32 + \
        (c1) * (arr).d32 + \
        (c2)])

#define WFC__A3D_GET_WRAP(arr, c0, c1, c2) \
    ((arr).a[ \
        wfc__indWrap(c0, (arr).d30) * (arr).d31 * (arr).d32 + \
        wfc__indWrap(c1, (arr).d31) * (arr).d32 + \
        wfc__indWrap(c2, (arr).d32)])

#define WFC__A4D_DEF(type, abbrv) \
    struct wfc__A4d_##abbrv { \
        int d40, d41, d42, d43; \
        type *a; \
    }

#define WFC__A4D_LEN(arr) ((arr).d40 * (arr).d41 * (arr).d42 * (arr).d43)

#define WFC__A4D_SIZE(arr) ((size_t)WFC__A4D_LEN(arr) * sizeof(*(arr).a))

#define WFC__A4D_GET(arr, c0, c1, c2, c3) \
    ((arr).a[ \
        (c0) * (arr).d41 * (arr).d42 * (arr).d43 + \
        (c1) * (arr).d42 * (arr).d43 + \
        (c2) * (arr).d43 + \
        (c3)])

#define WFC__A4D_GET_WRAP(arr, c0, c1, c2, c3) \
    ((arr).a[ \
        wfc__indWrap(c0, (arr).d40) * (arr).d41 * (arr).d42 * (arr).d43 + \
        wfc__indWrap(c1, (arr).d41) * (arr).d42 * (arr).d43 + \
        wfc__indWrap(c2, (arr).d42) * (arr).d43 + \
        wfc__indWrap(c3, (arr).d43)])

void wfc__indToCoords2d(int d1, int ind, int *c0, int *c1) {
    int c0_ = ind / d1;
    ind -= c0_ * d1;
    int c1_ = ind;

    *c0 = c0_;
    *c1 = c1_;
}

// wfc code

struct wfc__Pattern {
    int c0, c1;
    int freq;
};

WFC__A2D_DEF(uint8_t, u8);
WFC__A2D_DEF(float, f);
WFC__A3D_DEF(uint8_t, u8);
WFC__A3D_DEF(const uint8_t, cu8);
WFC__A4D_DEF(uint8_t, u8);

int wfc__patternsEq(
    int n, const struct wfc__A3d_cu8 src,
    struct wfc__Pattern patt1, struct wfc__Pattern patt2) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            const uint8_t *a = &WFC__A3D_GET_WRAP(src,
                patt1.c0 + i, patt1.c1 + j, 0);
            const uint8_t *b = &WFC__A3D_GET_WRAP(src,
                patt2.c0 + i, patt2.c1 + j, 0);

            if (memcmp(a, b, (size_t)src.d32) != 0) return 0;
        }
    }
    return 1;
}

struct wfc__Pattern* wfc__gatherPatterns(
    int n, const struct wfc__A3d_cu8 src, int *cnt) {
    struct wfc__Pattern *patts = NULL;

    int pattCnt = 0;
    for (int i = 0; i < src.d30 * src.d31; ++i) {
        struct wfc__Pattern patt = {0};
        wfc__indToCoords2d(src.d31, i, &patt.c0, &patt.c1);

        int seenBefore = 0;
        for (int j = 0; !seenBefore && j < i; ++j) {
            struct wfc__Pattern patt1 = {0};
            wfc__indToCoords2d(src.d31, j, &patt1.c0, &patt1.c1);

            if (wfc__patternsEq(n, src, patt, patt1)) seenBefore = 1;
        }

        if (!seenBefore) ++pattCnt;
    }

    patts = malloc((size_t)pattCnt * sizeof(*patts));
    pattCnt = 0;
    for (int i = 0; i < src.d30 * src.d31; ++i) {
        struct wfc__Pattern patt = {0};
        wfc__indToCoords2d(src.d31, i, &patt.c0, &patt.c1);
        patt.freq = 1;

        int seenBefore = 0;
        for (int j = 0; !seenBefore && j < pattCnt; ++j) {
            if (wfc__patternsEq(n, src, patt, patts[j])) {
                ++patts[j].freq;
                seenBefore = 1;
            }
        }

        if (!seenBefore) patts[pattCnt++] = patt;
    }

    *cnt = pattCnt;
    return patts;
}

int wfc__overlapMatches(
    int n, const struct wfc__A3d_cu8 src,
    int dc0, int dc1, struct wfc__Pattern patt1, struct wfc__Pattern patt2) {
    assert(abs(dc0) < n);
    assert(abs(dc1) < n);

    int overlapD0 = n - abs(dc0);
    int overlapD1 = n - abs(dc1);

    int overlapP1C0 = dc0 > 0 ? dc0 : 0;
    int overlapP1C1 = dc1 > 0 ? dc1 : 0;
    int overlapP2C0 = dc0 > 0 ? 0 : abs(dc0);
    int overlapP2C1 = dc1 > 0 ? 0 : abs(dc1);

    for (int i = 0; i < overlapD0; ++i) {
        for (int j = 0; j < overlapD1; ++j) {
            int p1c0 = overlapP1C0 + i;
            int p1c1 = overlapP1C1 + j;
            int p2c0 = overlapP2C0 + i;
            int p2c1 = overlapP2C1 + j;

            int m1c0 = patt1.c0 + p1c0;
            int m1c1 = patt1.c1 + p1c1;
            int m2c0 = patt2.c0 + p2c0;
            int m2c1 = patt2.c1 + p2c1;

            const uint8_t *a = &WFC__A3D_GET_WRAP(src, m1c0, m1c1, 0);
            const uint8_t *b = &WFC__A3D_GET_WRAP(src, m2c0, m2c1, 0);

            if (memcmp(a, b, (size_t)src.d32) != 0) return 0;
        }
    }

    return 1;
}

struct wfc__A4d_u8 wfc__calcOverlaps(
    int n, const struct wfc__A3d_cu8 src,
    int pattCnt, const struct wfc__Pattern *patts) {
    struct wfc__A4d_u8 overlaps;
    overlaps.d40 = 2 * n - 1;
    overlaps.d41 = 2 * n - 1;
    overlaps.d42 = pattCnt;
    overlaps.d43 = pattCnt;
    overlaps.a = malloc(WFC__A4D_SIZE(overlaps));

    for (int dc0 = -(n - 1); dc0 <= n - 1; ++dc0) {
        for (int dc1 = -(n - 1); dc1 <= n - 1; ++dc1) {
            for (int i = 0; i < pattCnt; ++i) {
                for (int j = 0; j < pattCnt; ++j) {
                    int overlap = wfc__overlapMatches(n, src,
                        dc0, dc1, patts[i], patts[j]);
                    WFC__A4D_GET(overlaps, dc0 + n - 1, dc1 + n - 1, i, j) =
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
    for (int c0 = 0; c0 < wave.d30; ++c0) {
        for (int c1 = 0; c1 < wave.d31; ++c1) {
            int totalFreq = 0;
            int availPatts = 0;
            for (int p = 0; p < wave.d32; ++p) {
                if (WFC__A3D_GET(wave, c0, c1, p)) {
                    totalFreq += patts[p].freq;
                    ++availPatts;
                }
            }

            float entropy = 0.0f;
            // check is here to ensure entropy of observed points becomes 0
            if (availPatts > 1) {
                for (int p = 0; p < wave.d32; ++p) {
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
        wfc__indToCoords2d(entropies.d21, chosenPnt, &chosenC0, &chosenC1);
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
    int n, int c0N, int c1N, int dc0, int dc1,
    const struct wfc__A4d_u8 overlaps,
    struct wfc__A3d_u8 wave) {
    int c0 = wfc__indWrap(c0N + dc0, wave.d30);
    int c1 = wfc__indWrap(c1N + dc1, wave.d31);
    int pattCnt = overlaps.d42;

    int oldAvailPattCnt = 0;
    for (int p = 0; p < pattCnt; ++p) {
        if (WFC__A3D_GET(wave, c0, c1, p)) ++oldAvailPattCnt;
    }

    for (int p = 0; p < pattCnt; ++p) {
        if (!WFC__A3D_GET(wave, c0, c1, p)) continue;

        uint8_t total = 0;
        for (int pN = 0; pN < pattCnt; ++pN) {
            total |= WFC__A3D_GET(wave, c0N, c1N, pN) &
                WFC__A4D_GET(overlaps, -dc0 + n - 1, -dc1 + n - 1, p, pN);
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
    int n, int c0N, int c1N,
    const struct wfc__A4d_u8 overlaps,
    struct wfc__A2d_u8 ripple,
    struct wfc__A3d_u8 wave) {
    int modified = 0;

    for (int dc0 = -(n - 1); dc0 <= n - 1; ++dc0) {
        for (int dc1 = -(n - 1); dc1 <= n - 1; ++dc1) {
            if (wfc__propagateOnto(n, c0N, c1N, dc0, dc1, overlaps, wave)) {
                WFC__A2D_GET_WRAP(ripple, c0N + dc0, c1N + dc1) = 1;
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

        for (int c0N = 0; c0N < wave.d30; ++c0N) {
            for (int c1N = 0; c1N < wave.d31; ++c1N) {
                if (!WFC__A2D_GET(ripple, c0N, c1N)) continue;

                if (wfc__propagateNeighbours(n, c0N, c1N,
                        overlaps, ripple, wave)) {
                    modified = 1;
                }

                WFC__A2D_GET(ripple, c0N, c1N) = 0;
            }
        }
    }
}

int wfc_generate(
    int n, int bytesPerPixel,
    int srcW, int srcH, const unsigned char *src,
    int dstW, int dstH, unsigned char *dst) {
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
    assert(dst != NULL);

    int ret = 0;

    struct wfc__Pattern *patts = NULL;
    struct wfc__A4d_u8 overlaps = {0};
    struct wfc__A3d_u8 wave = {0};
    struct wfc__A2d_f entropies = {0};
    struct wfc__A2d_u8 ripple = {0};

    struct wfc__A3d_cu8 srcA = {srcH, srcW, bytesPerPixel, src};
    struct wfc__A3d_u8 dstA = {dstH, dstW, bytesPerPixel, dst};

    int pattCnt;
    patts = wfc__gatherPatterns(n, srcA, &pattCnt);

    overlaps = wfc__calcOverlaps(n, srcA, pattCnt, patts);

    wave.d30 = dstH;
    wave.d31 = dstW;
    wave.d32 = pattCnt;
    wave.a = malloc(WFC__A3D_SIZE(wave));
    for (int i = 0; i < WFC__A3D_LEN(wave); ++i) wave.a[i] = 1;

    entropies.d20 = dstH;
    entropies.d21 = dstW;
    entropies.a = malloc(WFC__A2D_SIZE(entropies));

    ripple.d20 = dstH;
    ripple.d21 = dstW;
    ripple.a = malloc(WFC__A2D_SIZE(ripple));

    while (1) {
        int minPatts = pattCnt, maxPatts = 0;
        for (int c0 = 0; c0 < wave.d30; ++c0) {
            for (int c1 = 0; c1 < wave.d31; ++c1) {
                int patts = 0;
                for (int p = 0; p < wave.d32; ++p) {
                    if (WFC__A3D_GET(wave, c0, c1, p)) ++patts;
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

        int obsC0, obsC1;
        wfc__observeOne(pattCnt, patts, entropies, wave, &obsC0, &obsC1);

        wfc__propagate(n, obsC0, obsC1, overlaps, ripple, wave);
    }

    for (int c0 = 0; c0 < dstH; ++c0) {
        for (int c1 = 0; c1 < dstW; ++c1) {
            int patt = 0;
            for (int p = 0; p < pattCnt; ++p) {
                if (WFC__A3D_GET(wave, c0, c1, p)) {
                    patt = p;
                    break;
                }
            }

            int mc0 = patts[patt].c0;
            int mc1 = patts[patt].c1;

            const uint8_t *srcPx = &WFC__A3D_GET(srcA, mc0, mc1, 0);
            uint8_t *dstPx = &WFC__A3D_GET(dstA, c0, c1, 0);

            memcpy(dstPx, srcPx, (size_t)bytesPerPixel);
        }
    }

cleanup:
    free(ripple.a);
    free(entropies.a);
    free(wave.a);
    free(overlaps.a);
    free(patts);

    return ret;
}
