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

struct wfc__Size2d { int dim[2]; };
struct wfc__Size3d { int dim[3]; };
struct wfc__Size4d { int dim[4]; };

int wfc__len2d(struct wfc__Size2d sz) {
    return sz.dim[0] * sz.dim[1];
}

int wfc__len3d(struct wfc__Size3d sz) {
    return sz.dim[0] * sz.dim[1] * sz.dim[2];
}

int wfc__len4d(struct wfc__Size4d sz) {
    return sz.dim[0] * sz.dim[1] * sz.dim[2] * sz.dim[3];
}

size_t wfc__size2d(struct wfc__Size2d sz, size_t membSz) {
    return (size_t)wfc__len2d(sz) * membSz;
}

size_t wfc__size3d(struct wfc__Size3d sz, size_t membSz) {
    return (size_t)wfc__len3d(sz) * membSz;
}

size_t wfc__size4d(struct wfc__Size4d sz, size_t membSz) {
    return (size_t)wfc__len4d(sz) * membSz;
}

int wfc__coordsToInd2d(struct wfc__Size2d sz, int c0, int c1) {
    return c1 * sz.dim[0] + c0;
}

int wfc__coordsToInd3d(struct wfc__Size3d sz, int c0, int c1, int c2) {
    return c2 * sz.dim[0] * sz.dim[1] + c1 * sz.dim[0] + c0;
}

int wfc__coordsToInd4d(struct wfc__Size4d sz, int c0, int c1, int c2, int c3) {
    return c3 * sz.dim[0] * sz.dim[1] * sz.dim[2]
        + c2 * sz.dim[0] * sz.dim[1] + c1 * sz.dim[0] + c0;
}

int wfc__coordsToInd2dWrap(struct wfc__Size2d sz, int c0, int c1) {
    return wfc__coordsToInd2d(
        sz,
        wfc__indWrap(c0, sz.dim[0]),
        wfc__indWrap(c1, sz.dim[1]));
}

int wfc__coordsToInd3dWrap(struct wfc__Size3d sz, int c0, int c1, int c2) {
    return wfc__coordsToInd3d(
        sz,
        wfc__indWrap(c0, sz.dim[0]),
        wfc__indWrap(c1, sz.dim[1]),
        wfc__indWrap(c2, sz.dim[2]));
}

int wfc__coordsToInd4dWrap(struct wfc__Size4d sz,
    int c0, int c1, int c2, int c3) {
    return wfc__coordsToInd4d(
        sz,
        wfc__indWrap(c0, sz.dim[0]),
        wfc__indWrap(c1, sz.dim[1]),
        wfc__indWrap(c2, sz.dim[2]),
        wfc__indWrap(c3, sz.dim[3]));
}

void wfc__indToCoords2d(struct wfc__Size2d sz, int ind, int *c0, int *c1) {
    int c1_ = ind / sz.dim[0];
    ind -= c1_ * sz.dim[0];
    int c0_ = ind;

    *c0 = c0_;
    *c1 = c1_;
}

void wfc__indToCoords3d(
    struct wfc__Size3d sz, int ind, int *c0, int *c1, int *c2) {
    int c2_ = ind / (sz.dim[0] * sz.dim[1]);
    ind -= c2_ * (sz.dim[0] * sz.dim[1]);
    int c1_ = ind / sz.dim[0];
    ind -= c1_ * sz.dim[0];
    int c0_ = ind;

    *c0 = c0_;
    *c1 = c1_;
    *c2 = c2_;
}

void wfc__indToCoords4d(
    struct wfc__Size4d sz, int ind, int *c0, int *c1, int *c2, int *c3) {
    int c3_ = ind / (sz.dim[0] * sz.dim[1] * sz.dim[2]);
    ind -= c3_ * (sz.dim[0] * sz.dim[1] * sz.dim[2]);
    int c2_ = ind / (sz.dim[0] * sz.dim[1]);
    ind -= c2_ * (sz.dim[0] * sz.dim[1]);
    int c1_ = ind / sz.dim[0];
    ind -= c1_ * sz.dim[0];
    int c0_ = ind;

    *c0 = c0_;
    *c1 = c1_;
    *c2 = c2_;
    *c3 = c3_;
}

// wfc code

struct wfc__Pattern {
    int l, t;
    int freq;
};

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

void wfc__calcEntropies(
    const struct wfc__Pattern *patts,
    struct wfc__Size3d waveSz, uint8_t *wave,
    float *entropies) {
    struct wfc__Size2d dstSz = {{waveSz.dim[0], waveSz.dim[1]}};

    for (int x = 0; x < waveSz.dim[0]; ++x) {
        for (int y = 0; y < waveSz.dim[1]; ++y) {
            int totalFreq = 0;
            int availPatts = 0;
            for (int z = 0; z < waveSz.dim[2]; ++z) {
                if (wave[wfc__coordsToInd3d(waveSz, x, y, z)]) {
                    totalFreq += patts[z].freq;
                    ++availPatts;
                }
            }

            float entropy = 0.0f;
            // check is here to ensure entropy of observed points becomes 0
            if (availPatts > 1) {
                for (int z = 0; z < waveSz.dim[2]; ++z) {
                    if (wave[wfc__coordsToInd3d(waveSz, x, y, z)]) {
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
    struct wfc__Size3d waveSz, uint8_t *wave) {
    struct wfc__Size2d dstSz = {{waveSz.dim[0], waveSz.dim[1]}};

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
            if (wave[wfc__coordsToInd3d(waveSz, chosenX, chosenY, i)]) {
                totalFreq += patts[i].freq;
            }
        }
        int chosenInst = wfc__rand_i(totalFreq);

        for (int i = 0; i < pattCnt; ++i) {
            if (wave[wfc__coordsToInd3d(waveSz, chosenX, chosenY, i)]) {
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
        wave[wfc__coordsToInd3d(waveSz, chosenX, chosenY, i)] = 0;
    }
    wave[wfc__coordsToInd3d(waveSz, chosenX, chosenY, chosenPatt)] = 1;
}

int wfc__overlapMatches(
    int n,
    struct wfc__Size2d srcSz, const uint32_t *src,
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

            uint32_t a = src[wfc__coordsToInd2dWrap(srcSz, mx1, my1)];
            uint32_t b = src[wfc__coordsToInd2dWrap(srcSz, mx2, my2)];

            if (a != b) return 0;
        }
    }

    return 1;
}

int wfc__propagateSingle(
    int n,
    struct wfc__Size2d srcSz, const uint32_t *src,
    int x, int y, int xN, int yN,
    int pattCnt, const struct wfc__Pattern *patts,
    struct wfc__Size3d waveSz, uint8_t *wave) {
    int changed = 0;

    for (int p = 0; p < pattCnt; ++p) {
        if (wave[wfc__coordsToInd3dWrap(waveSz, x, y, p)]) {
            int mayKeep = 0;
            for (int pN = 0; pN < pattCnt; ++pN) {
                if (wave[wfc__coordsToInd3dWrap(waveSz, xN, yN, pN)]) {
                    if (wfc__overlapMatches(n, srcSz, src,
                            x, y, xN, yN, patts[p], patts[pN])) {
                        mayKeep = 1;
                        break;
                    }
                }
            }

            if (!mayKeep) {
                wave[wfc__coordsToInd3dWrap(waveSz, x, y, p)] = 0;
                changed = 1;
            }
        }
    }

    return changed;
}

void wfc__propagate(
    int n,
    struct wfc__Size2d srcSz, const uint32_t *src,
    int pattCnt, const struct wfc__Pattern *patts,
    int seedX, int seedY,
    uint8_t *ripple,
    struct wfc__Size3d waveSz, uint8_t *wave) {
    struct wfc__Size2d dstSz = {{waveSz.dim[0], waveSz.dim[1]}};

    memset(ripple, 0, wfc__size2d(dstSz, sizeof(*ripple)));
    ripple[wfc__coordsToInd2d(dstSz, seedX, seedY)] = 1;

    uint8_t oddEven = 0;

    while (1) {
        uint8_t oddEvenMask = (uint8_t)(1 << oddEven);
        uint8_t oddEvenMaskNext = (uint8_t)(1 << (1 - oddEven));

        int done = 1;
        for (int x = 0; x < dstSz.dim[0]; ++x) {
            for (int y = 0; y < dstSz.dim[1]; ++y) {
                if (ripple[wfc__coordsToInd2d(dstSz, x, y)] & oddEvenMask) {
                    done = 0;
                }
                ripple[wfc__coordsToInd2d(dstSz, x, y)] &= ~oddEvenMaskNext;
            }
        }

        if (done) break;

        for (int xN = 0; xN < waveSz.dim[0]; ++xN) {
            for (int yN = 0; yN < waveSz.dim[1]; ++yN) {
                if (!(ripple[wfc__coordsToInd2d(dstSz, xN, yN)] &
                        oddEvenMask)) {
                    continue;
                }

                for (int dx = -(n - 1); dx <= n - 1; ++dx) {
                    for (int dy = -(n - 1); dy <= n - 1; ++dy) {
                        int x = xN + dx;
                        int y = yN + dy;

                        if (wfc__propagateSingle(n, srcSz, src,
                                x, y, xN, yN, pattCnt, patts, waveSz, wave)) {
                            ripple[wfc__coordsToInd2dWrap(dstSz, x, y)]
                                |= oddEvenMask | oddEvenMaskNext;
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
    uint8_t *wave = NULL;
    float *entropies = NULL;
    uint8_t *ripple = NULL;

    struct wfc__Size2d srcSz = {{srcW, srcH}};
    struct wfc__Size2d dstSz = {{dstW, dstH}};

    int pattCnt;
    patts = wfc__gatherPatterns(n, srcSz, src, &pattCnt);

    struct wfc__Size3d waveSz = {{dstW, dstH, pattCnt}};
    wave = malloc(wfc__size3d(waveSz, sizeof(*wave)));
    for (int i = 0; i < wfc__len3d(waveSz); ++i) wave[i] = 1;

    entropies = malloc(wfc__size2d(dstSz, sizeof(*entropies)));
    ripple = malloc(wfc__size2d(dstSz, sizeof(*ripple)));

    while (1) {
        int minPatts = pattCnt, maxPatts = 0;
        for (int x = 0; x < waveSz.dim[0]; ++x) {
            for (int y = 0; y < waveSz.dim[1]; ++y) {
                int patts = 0;
                for (int z = 0; z < waveSz.dim[2]; ++z) {
                    if (wave[wfc__coordsToInd3d(waveSz, x, y, z)]) ++patts;
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

        wfc__calcEntropies(patts, waveSz, wave, entropies);

        int obsX, obsY;
        wfc__observeOne(pattCnt, patts, entropies,
            &obsX, &obsY, waveSz, wave);

        wfc__propagate(n, srcSz, src, pattCnt, patts, obsX, obsY,
            ripple, waveSz, wave);
    }

    for (int x = 0; x < dstW; ++x) {
        for (int y = 0; y < dstH; ++y) {
            int patt = 0;
            for (int z = 0; z < pattCnt; ++z) {
                if (wave[wfc__coordsToInd3d(waveSz, x, y, z)]) {
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
    free(ripple);
    free(entropies);
    free(wave);
    free(patts);

    return ret;
}
