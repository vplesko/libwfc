#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "testing.h"
#include "wfc.h"

int testBasicN1(void) {
    enum { n = 1, srcW = 4, srcH = 4, dstW = 16, dstH = 16 };

    uint32_t src[srcW * srcH] = {
        5,5,5,5,
        5,5,6,5,
        5,6,6,5,
        5,5,5,5,
    };
    uint32_t dst[dstW * dstH];

    if (wfc_generate(
        n,
        srcW, srcH, (uint32_t*)&src,
        dstW, dstH, (uint32_t*)&dst) != 0) {
        PRINT_TEST_FAIL();
        return 1;
    }

    for (int i = 0; i < dstW * dstH; ++i) {
        if (dst[i] != 5 && dst[i] != 6) {
            PRINT_TEST_FAIL();
            return 1;
        }
    }

    return 0;
}

int testBasicN3(void) {
    enum { n = 3, srcW = 4, srcH = 4, dstW = 16, dstH = 16 };

    uint32_t src[srcW * srcH] = {
        5,5,5,5,
        5,5,6,5,
        5,6,6,5,
        5,5,5,5,
    };
    uint32_t dst[dstW * dstH];

    if (wfc_generate(
        n,
        srcW, srcH, (uint32_t*)&src,
        dstW, dstH, (uint32_t*)&dst) != 0) {
        PRINT_TEST_FAIL();
        return 1;
    }

    for (int i = 0; i < dstW * dstH; ++i) {
        if (dst[i] != 5 && dst[i] != 6) {
            PRINT_TEST_FAIL();
            return 1;
        }
    }

    return 0;
}

int testPattern(void) {
    enum { n = 2, srcW = 3, srcH = 3, dstW = 32, dstH = 32 };

    uint32_t src[srcH][srcW] = {
        {0,1,0},
        {1,2,1},
        {0,1,0},
    };
    uint32_t dst[dstH][dstW];

    if (wfc_generate(
        n,
        srcW, srcH, (uint32_t*)&src,
        dstW, dstH, (uint32_t*)&dst) != 0) {
        PRINT_TEST_FAIL();
        return 1;
    }

    for (int y = 0; y < dstH; ++y) {
        for (int x = 0; x < dstW; ++x) {
            if (dst[y][x] != 0 && dst[y][x] != 1 && dst[y][x] != 2) {
                PRINT_TEST_FAIL();
                return 1;
            }
            if (dst[y][x] == 2) {
                int l = x > 0 ? x - 1 : dstW - 1;
                int r = (x + 1) % dstW;
                int u = y > 0 ? y - 1 : dstH - 1;
                int d = (y + 1) % dstH;

                if (dst[y][l] == 0 || dst[y][r] == 0 ||
                    dst[u][x] == 0 || dst[d][x] == 0) {
                    PRINT_TEST_FAIL();
                    return 1;
                }
            }
        }
    }

    return 0;
}

int testWide(void) {
    enum { n = 2, srcW = 6, srcH = 4, dstW = 32, dstH = 16 };

    uint32_t src[srcW * srcH] = {
        0,0,0,0,0,0,
        0,1,1,1,1,0,
        0,1,1,1,1,0,
        0,0,0,0,0,0,
    };
    uint32_t dst[dstW * dstH];

    if (wfc_generate(
        n,
        srcW, srcH, (uint32_t*)&src,
        dstW, dstH, (uint32_t*)&dst) != 0) {
        PRINT_TEST_FAIL();
        return 1;
    }

    for (int i = 0; i < dstW * dstH; ++i) {
        if (dst[i] != 0 && dst[i] != 1) {
            PRINT_TEST_FAIL();
            return 1;
        }
    }

    return 0;
}

int testTall(void) {
    enum { n = 2, srcW = 4, srcH = 6, dstW = 16, dstH = 32 };

    uint32_t src[srcW * srcH] = {
        0,0,0,0,
        0,1,1,0,
        0,1,1,0,
        0,1,1,0,
        0,1,1,0,
        0,0,0,0,
    };
    uint32_t dst[dstW * dstH];

    if (wfc_generate(
        n,
        srcW, srcH, (uint32_t*)&src,
        dstW, dstH, (uint32_t*)&dst) != 0) {
        PRINT_TEST_FAIL();
        return 1;
    }

    for (int i = 0; i < dstW * dstH; ++i) {
        if (dst[i] != 0 && dst[i] != 1) {
            PRINT_TEST_FAIL();
            return 1;
        }
    }

    return 0;
}

// @TODO add a test for when src is bigger than dst
int main(void) {
    unsigned seed = (unsigned)time(NULL);
    srand(seed);

    if (testBasicN1() != 0 ||
        testBasicN3() != 0 ||
        testPattern() != 0 ||
        testWide() != 0 ||
        testTall() != 0) {
        printf("Seed was: %u\n", seed);
        return 1;
    }

    return 0;
}
