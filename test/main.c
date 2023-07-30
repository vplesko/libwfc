#include "wfc.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "testing.h"

// tests functions are static to make it a compile error if any are not called
static int testBasicN1(void) {
    enum { n = 1, srcW = 4, srcH = 4, dstW = 16, dstH = 16 };

    uint32_t src[srcW * srcH] = {
        5,5,5,5,
        5,5,6,5,
        5,6,6,5,
        5,5,5,5,
    };
    uint32_t dst[dstW * dstH];

    if (wfc_generate(
        n, 0, sizeof(*src),
        srcW, srcH, (unsigned char*)&src,
        dstW, dstH, (unsigned char*)&dst) != 0) {
        PRINT_TEST_FAIL();
        return -1;
    }

    for (int i = 0; i < dstW * dstH; ++i) {
        if (dst[i] != 5 && dst[i] != 6) {
            PRINT_TEST_FAIL();
            return -1;
        }
    }

    return 0;
}

static int testHFlipN1(void) {
    enum { n = 1, srcW = 4, srcH = 4, dstW = 16, dstH = 16 };

    uint32_t src[srcW * srcH] = {
        5,5,5,5,
        5,5,6,5,
        5,6,6,5,
        5,5,5,5,
    };
    uint32_t dst[dstW * dstH];

    if (wfc_generate(
        n, wfc_optFlipH, sizeof(*src),
        srcW, srcH, (unsigned char*)&src,
        dstW, dstH, (unsigned char*)&dst) != 0) {
        PRINT_TEST_FAIL();
        return -1;
    }

    for (int i = 0; i < dstW * dstH; ++i) {
        if (dst[i] != 5 && dst[i] != 6) {
            PRINT_TEST_FAIL();
            return -1;
        }
    }

    return 0;
}

static int testVFlipN1(void) {
    enum { n = 1, srcW = 4, srcH = 4, dstW = 16, dstH = 16 };

    uint32_t src[srcW * srcH] = {
        5,5,5,5,
        5,5,6,5,
        5,6,6,5,
        5,5,5,5,
    };
    uint32_t dst[dstW * dstH];

    if (wfc_generate(
        n, wfc_optFlipV, sizeof(*src),
        srcW, srcH, (unsigned char*)&src,
        dstW, dstH, (unsigned char*)&dst) != 0) {
        PRINT_TEST_FAIL();
        return -1;
    }

    for (int i = 0; i < dstW * dstH; ++i) {
        if (dst[i] != 5 && dst[i] != 6) {
            PRINT_TEST_FAIL();
            return -1;
        }
    }

    return 0;
}

static int testHVFlipN1(void) {
    enum { n = 1, srcW = 4, srcH = 4, dstW = 16, dstH = 16 };

    uint32_t src[srcW * srcH] = {
        5,5,5,5,
        5,5,6,5,
        5,6,6,5,
        5,5,5,5,
    };
    uint32_t dst[dstW * dstH];

    if (wfc_generate(
        n, wfc_optFlipH | wfc_optFlipV, sizeof(*src),
        srcW, srcH, (unsigned char*)&src,
        dstW, dstH, (unsigned char*)&dst) != 0) {
        PRINT_TEST_FAIL();
        return -1;
    }

    for (int i = 0; i < dstW * dstH; ++i) {
        if (dst[i] != 5 && dst[i] != 6) {
            PRINT_TEST_FAIL();
            return -1;
        }
    }

    return 0;
}

static int testBasicN3(void) {
    enum { n = 3, srcW = 4, srcH = 4, dstW = 16, dstH = 16 };

    uint32_t src[srcW * srcH] = {
        5,5,5,5,
        5,5,6,5,
        5,6,6,5,
        5,5,5,5,
    };
    uint32_t dst[dstW * dstH];

    if (wfc_generate(
        n, 0, sizeof(*src),
        srcW, srcH, (unsigned char*)&src,
        dstW, dstH, (unsigned char*)&dst) != 0) {
        PRINT_TEST_FAIL();
        return -1;
    }

    for (int i = 0; i < dstW * dstH; ++i) {
        if (dst[i] != 5 && dst[i] != 6) {
            PRINT_TEST_FAIL();
            return -1;
        }
    }

    return 0;
}

static int testHFlipN3(void) {
    enum { n = 3, srcW = 4, srcH = 4, dstW = 16, dstH = 16 };

    uint32_t src[srcW * srcH] = {
        5,5,5,5,
        5,6,6,5,
        5,6,6,5,
        5,5,5,5,
    };
    uint32_t dst[dstW * dstH];

    if (wfc_generate(
        n, wfc_optFlipH, sizeof(*src),
        srcW, srcH, (unsigned char*)&src,
        dstW, dstH, (unsigned char*)&dst) != 0) {
        PRINT_TEST_FAIL();
        return -1;
    }

    for (int i = 0; i < dstW * dstH; ++i) {
        if (dst[i] != 5 && dst[i] != 6) {
            PRINT_TEST_FAIL();
            return -1;
        }
    }

    return 0;
}

static int testVFlipN3(void) {
    enum { n = 3, srcW = 4, srcH = 4, dstW = 16, dstH = 16 };

    uint32_t src[srcW * srcH] = {
        5,5,5,5,
        5,6,6,5,
        5,6,6,5,
        5,5,5,5,
    };
    uint32_t dst[dstW * dstH];

    if (wfc_generate(
        n, wfc_optFlipV, sizeof(*src),
        srcW, srcH, (unsigned char*)&src,
        dstW, dstH, (unsigned char*)&dst) != 0) {
        PRINT_TEST_FAIL();
        return -1;
    }

    for (int i = 0; i < dstW * dstH; ++i) {
        if (dst[i] != 5 && dst[i] != 6) {
            PRINT_TEST_FAIL();
            return -1;
        }
    }

    return 0;
}

static int testHVFlipN3(void) {
    enum { n = 3, srcW = 4, srcH = 4, dstW = 16, dstH = 16 };

    uint32_t src[srcW * srcH] = {
        5,5,5,5,
        5,6,6,5,
        5,6,6,5,
        5,5,5,5,
    };
    uint32_t dst[dstW * dstH];

    if (wfc_generate(
        n, wfc_optFlipH | wfc_optFlipV, sizeof(*src),
        srcW, srcH, (unsigned char*)&src,
        dstW, dstH, (unsigned char*)&dst) != 0) {
        PRINT_TEST_FAIL();
        return -1;
    }

    for (int i = 0; i < dstW * dstH; ++i) {
        if (dst[i] != 5 && dst[i] != 6) {
            PRINT_TEST_FAIL();
            return -1;
        }
    }

    return 0;
}

static int testVEdgeFixN3(void) {
    enum { n = 3, srcW = 5, srcH = 5, dstW = 16, dstH = 16 };

    uint32_t src[srcW * srcH] = {
        1,1,1,1,1,
        3,3,3,3,3,
        3,3,3,3,3,
        3,3,3,3,3,
        2,2,2,2,2,
    };
    uint32_t dst[dstW * dstH];

    if (wfc_generate(
        n, wfc_optEdgeFixV, sizeof(*src),
        srcW, srcH, (unsigned char*)&src,
        dstW, dstH, (unsigned char*)&dst) != 0) {
        PRINT_TEST_FAIL();
        return -1;
    }

    for (int i = 0; i < dstW * dstH; ++i) {
        if (dst[i] < 1 || dst[i] > 3) {
            PRINT_TEST_FAIL();
            return -1;
        }
    }

    return 0;
}

static int testHEdgeFixN3(void) {
    enum { n = 3, srcW = 5, srcH = 5, dstW = 16, dstH = 16 };

    uint32_t src[srcW * srcH] = {
        1,3,3,3,2,
        1,3,3,3,2,
        1,3,3,3,2,
        1,3,3,3,2,
        1,3,3,3,2,
    };
    uint32_t dst[dstW * dstH];

    if (wfc_generate(
        n, wfc_optEdgeFixH, sizeof(*src),
        srcW, srcH, (unsigned char*)&src,
        dstW, dstH, (unsigned char*)&dst) != 0) {
        PRINT_TEST_FAIL();
        return -1;
    }

    for (int i = 0; i < dstW * dstH; ++i) {
        if (dst[i] < 1 || dst[i] > 3) {
            PRINT_TEST_FAIL();
            return -1;
        }
    }

    return 0;
}

static int testHVEdgeFixN3(void) {
    enum { n = 3, srcW = 5, srcH = 5, dstW = 16, dstH = 16 };

    uint32_t src[srcW * srcH] = {
        1,1,1,1,1,
        1,2,2,2,1,
        1,2,2,2,1,
        1,2,2,2,1,
        1,1,1,1,1,
    };
    uint32_t dst[dstW * dstH];

    if (wfc_generate(
        n, wfc_optEdgeFixH | wfc_optEdgeFixV, sizeof(*src),
        srcW, srcH, (unsigned char*)&src,
        dstW, dstH, (unsigned char*)&dst) != 0) {
        PRINT_TEST_FAIL();
        return -1;
    }

    for (int i = 0; i < dstW * dstH; ++i) {
        if (dst[i] < 1 || dst[i] > 2) {
            PRINT_TEST_FAIL();
            return -1;
        }
    }

    return 0;
}

static int testHVEdgeFixOneSolution(void) {
    enum { n = 2, srcW = 4, srcH = 4, dstW = 5, dstH = 5 };

    uint32_t src[srcW * srcH] = {
        1,1,1,2,
        4,5,5,2,
        4,5,5,2,
        4,3,3,3,
    };
    uint32_t dst[dstW * dstH];

    if (wfc_generate(
        n, wfc_optEdgeFixH | wfc_optEdgeFixV, sizeof(*src),
        srcW, srcH, (unsigned char*)&src,
        dstW, dstH, (unsigned char*)&dst) != 0) {
        PRINT_TEST_FAIL();
        return -1;
    }

    uint32_t expected[dstW * dstH] = {
        1,1,1,1,2,
        4,5,5,5,2,
        4,5,5,5,2,
        4,5,5,5,2,
        4,3,3,3,3,
    };
    for (int i = 0; i < dstW * dstH; ++i) {
        if (dst[i] != expected[i]) {
            PRINT_TEST_FAIL();
            return -1;
        }
    }

    return 0;
}

static int testPattern(void) {
    enum { n = 2, srcW = 3, srcH = 3, dstW = 32, dstH = 32 };

    uint32_t src[srcW * srcH] = {
        0,1,0,
        1,2,1,
        0,1,0,
    };
    uint32_t dst[dstW * dstH];

    if (wfc_generate(
        n, 0, sizeof(*src),
        srcW, srcH, (unsigned char*)&src,
        dstW, dstH, (unsigned char*)&dst) != 0) {
        PRINT_TEST_FAIL();
        return -1;
    }

    for (int i = 0; i < dstW * dstH; ++i) {
        int y = i / dstW;
        int x = i % dstW;

        if (dst[i] != 0 && dst[i] != 1 && dst[i] != 2) {
            PRINT_TEST_FAIL();
            return -1;
        }
        if (dst[i] == 2) {
            int l = x > 0 ? x - 1 : dstW - 1;
            int r = (x + 1) % dstW;
            int u = y > 0 ? y - 1 : dstH - 1;
            int d = (y + 1) % dstH;

            if (dst[y * dstW + l] == 0 || dst[y * dstW + r] == 0 ||
                dst[u * dstW + x] == 0 || dst[d * dstW + x] == 0) {
                PRINT_TEST_FAIL();
                return -1;
            }
        }
    }

    return 0;
}

static int testHVEdgeFixPattern(void) {
    enum { n = 2, srcW = 4, srcH = 4, dstW = 32, dstH = 32 };

    uint32_t src[srcW * srcH] = {
        0,0,0,0,
        0,1,1,0,
        0,1,1,0,
        0,0,0,0,
    };
    uint32_t dst[dstW * dstH];

    int options = wfc_optEdgeFixH | wfc_optEdgeFixV |
        wfc_optFlipH | wfc_optFlipV | wfc_optRotate;

    if (wfc_generate(
        n, options, sizeof(*src),
        srcW, srcH, (unsigned char*)&src,
        dstW, dstH, (unsigned char*)&dst) != 0) {
        PRINT_TEST_FAIL();
        return -1;
    }

    for (int i = 0; i < dstW; ++i) {
        if (dst[0 * dstW + i] != 0 || dst[(dstH - 1) * dstW + i] != 0) {
            PRINT_TEST_FAIL();
            return -1;
        }
    }
    for (int i = 0; i < dstH; ++i) {
        if (dst[i * dstW + 0] != 0 || dst[i * dstW + (dstW - 1)] != 0) {
            PRINT_TEST_FAIL();
            return -1;
        }
    }

    return 0;
}

static int testWide(void) {
    enum { n = 2, srcW = 6, srcH = 4, dstW = 32, dstH = 16 };

    uint32_t src[srcW * srcH] = {
        0,0,0,0,0,0,
        0,1,1,1,1,0,
        0,1,1,1,1,0,
        0,0,0,0,0,0,
    };
    uint32_t dst[dstW * dstH];

    if (wfc_generate(
        n, wfc_optFlipH | wfc_optFlipV, sizeof(*src),
        srcW, srcH, (unsigned char*)&src,
        dstW, dstH, (unsigned char*)&dst) != 0) {
        PRINT_TEST_FAIL();
        return -1;
    }

    for (int i = 0; i < dstW * dstH; ++i) {
        if (dst[i] != 0 && dst[i] != 1) {
            PRINT_TEST_FAIL();
            return -1;
        }
    }

    return 0;
}

static int testTall(void) {
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
        n, wfc_optFlipH | wfc_optFlipV, sizeof(*src),
        srcW, srcH, (unsigned char*)&src,
        dstW, dstH, (unsigned char*)&dst) != 0) {
        PRINT_TEST_FAIL();
        return -1;
    }

    for (int i = 0; i < dstW * dstH; ++i) {
        if (dst[i] != 0 && dst[i] != 1) {
            PRINT_TEST_FAIL();
            return -1;
        }
    }

    return 0;
}

static int testSrcBiggerThanDst(void) {
    enum { n = 2, srcW = 6, srcH = 6, dstW = 4, dstH = 4 };

    uint32_t src[srcW * srcH] = {
        0,0,0,0,0,0,
        0,1,1,1,1,0,
        0,1,1,1,1,0,
        0,1,1,1,1,0,
        0,1,1,1,1,0,
        0,0,0,0,0,0
    };
    uint32_t dst[dstW * dstH];

    if (wfc_generate(
        n, wfc_optFlipH | wfc_optFlipV, sizeof(*src),
        srcW, srcH, (unsigned char*)&src,
        dstW, dstH, (unsigned char*)&dst) != 0) {
        PRINT_TEST_FAIL();
        return -1;
    }

    for (int i = 0; i < dstW * dstH; ++i) {
        if (dst[i] != 0 && dst[i] != 1) {
            PRINT_TEST_FAIL();
            return -1;
        }
    }

    return 0;
}

static int testPatternCountBasic(void) {
    enum { n = 2, srcW = 3, srcH = 3 };

    int ret = 0;

    wfc_State *state = NULL;

    uint32_t src[srcW * srcH] = {
        5,5,5,
        5,5,6,
        5,6,6,
    };

    state = wfc_init(n, 0, sizeof(*src),
        srcW, srcH, (unsigned char*)&src,
        16, 16);

    if (wfc_patternCount(state) != 8) {
        PRINT_TEST_FAIL();
        ret = 1;
        goto cleanup;
    }

cleanup:
    wfc_free(state);

    return ret;
}

static int testPatternCountHFlip(void) {
    enum { n = 2, srcW = 3, srcH = 2 };

    int ret = 0;

    wfc_State *state = NULL;

    uint32_t src[srcW * srcH] = {
        1,2,1,
        3,4,3,
    };

    state = wfc_init(n, wfc_optFlipH, sizeof(*src),
        srcW, srcH, (unsigned char*)&src,
        16, 16);

    if (wfc_patternCount(state) != 6) {
        PRINT_TEST_FAIL();
        ret = 1;
        goto cleanup;
    }

cleanup:
    wfc_free(state);

    return ret;
}

static int testPatternCountVFlip(void) {
    enum { n = 2, srcW = 2, srcH = 3 };

    int ret = 0;

    wfc_State *state = NULL;

    uint32_t src[srcW * srcH] = {
        1,2,
        3,4,
        1,2,
    };

    state = wfc_init(n, wfc_optFlipV, sizeof(*src),
        srcW, srcH, (unsigned char*)&src,
        16, 16);

    if (wfc_patternCount(state) != 6) {
        PRINT_TEST_FAIL();
        ret = 1;
        goto cleanup;
    }

cleanup:
    wfc_free(state);

    return ret;
}

static int testPatternCountHVFlip(void) {
    enum { n = 2, srcW = 2, srcH = 2 };

    int ret = 0;

    wfc_State *state = NULL;

    uint32_t src[srcW * srcH] = {
        1,2,
        2,1,
    };

    state = wfc_init(n, wfc_optFlipV, sizeof(*src),
        srcW, srcH, (unsigned char*)&src,
        16, 16);

    if (wfc_patternCount(state) != 2) {
        PRINT_TEST_FAIL();
        ret = 1;
        goto cleanup;
    }

cleanup:
    wfc_free(state);

    return ret;
}

static int testPatternCountRotate(void) {
    enum { n = 2, srcW = 4, srcH = 4 };

    int ret = 0;

    wfc_State *state = NULL;

    uint32_t src[srcW * srcH] = {
        0,0,0,0,
        0,0,1,0,
        0,1,1,0,
        0,0,0,0,
    };

    state = wfc_init(n, wfc_optRotate, sizeof(*src),
        srcW, srcH, (unsigned char*)&src,
        16, 16);

    if (wfc_patternCount(state) != 13) {
        PRINT_TEST_FAIL();
        ret = 1;
        goto cleanup;
    }

cleanup:
    wfc_free(state);

    return ret;
}

static int testPatternCountRotateCentralSymmetric(void) {
    enum { n = 2, srcW = 2, srcH = 2 };

    int ret = 0;

    wfc_State *state = NULL;

    uint32_t src[srcW * srcH] = {
        1,2,
        2,1,
    };

    state = wfc_init(n, wfc_optRotate, sizeof(*src),
        srcW, srcH, (unsigned char*)&src,
        16, 16);

    if (wfc_patternCount(state) != 2) {
        PRINT_TEST_FAIL();
        ret = 1;
        goto cleanup;
    }

cleanup:
    wfc_free(state);

    return ret;
}

static int testPatternCountHFlipRotate(void) {
    enum { n = 2, srcW = 3, srcH = 3 };

    int ret = 0;

    wfc_State *state = NULL;

    uint32_t src[srcW * srcH] = {
        1,1,3,
        2,2,3,
        3,3,3,
    };

    state = wfc_init(n, wfc_optFlipH | wfc_optRotate, sizeof(*src),
        srcW, srcH, (unsigned char*)&src,
        16, 16);

    if (wfc_patternCount(state) != 28) {
        PRINT_TEST_FAIL();
        ret = 1;
        goto cleanup;
    }

cleanup:
    wfc_free(state);

    return ret;
}

static int testPatternCountVFlipRotate(void) {
    enum { n = 2, srcW = 3, srcH = 3 };

    int ret = 0;

    wfc_State *state = NULL;

    uint32_t src[srcW * srcH] = {
        1,1,3,
        2,2,3,
        3,3,3,
    };

    state = wfc_init(n, wfc_optFlipV | wfc_optRotate, sizeof(*src),
        srcW, srcH, (unsigned char*)&src,
        16, 16);

    if (wfc_patternCount(state) != 28) {
        PRINT_TEST_FAIL();
        ret = 1;
        goto cleanup;
    }

cleanup:
    wfc_free(state);

    return ret;
}

static int testPatternCountHVFlipRotate(void) {
    enum { n = 2, srcW = 2, srcH = 2 };

    int ret = 0;

    wfc_State *state = NULL;

    uint32_t src[srcW * srcH] = {
        1,2,
        3,4,
    };

    state = wfc_init(
        n, wfc_optFlipH | wfc_optFlipV | wfc_optRotate, sizeof(*src),
        srcW, srcH, (unsigned char*)&src,
        16, 16);

    if (wfc_patternCount(state) != 8) {
        PRINT_TEST_FAIL();
        ret = 1;
        goto cleanup;
    }

cleanup:
    wfc_free(state);

    return ret;
}

static int testClone(void) {
    enum { n = 3, srcW = 4, srcH = 4, dstW = 16, dstH = 16 };

    int ret = 0;

    wfc_State *clone = NULL;

    uint32_t src[srcW * srcH] = {
        5,5,5,5,
        5,5,6,5,
        5,6,6,5,
        5,5,5,5,
    };
    uint32_t dst[dstW * dstH];

    {
        wfc_State *state = wfc_init(
            n, 0, sizeof(*src),
            srcW, srcH, (unsigned char*)&src,
            dstW, dstH);

        clone = wfc_clone(state);

        wfc_free(state);
    }

    while (!wfc_step(clone));
    if (wfc_status(clone) <= 0) {
        PRINT_TEST_FAIL();
        ret = 1;
        goto cleanup;
    }

    wfc_blit(clone, (unsigned char*)&src, (unsigned char*)&dst);
    for (int i = 0; i < dstW * dstH; ++i) {
        if (dst[i] != 5 && dst[i] != 6) {
            PRINT_TEST_FAIL();
            ret = 1;
            goto cleanup;
        }
    }

cleanup:
    wfc_free(clone);

    return ret;
}

int main(void) {
    unsigned seed = (unsigned)time(NULL);
    srand(seed);

    if (testBasicN1() != 0 ||
        testHFlipN1() != 0 ||
        testVFlipN1() != 0 ||
        testHVFlipN1() != 0 ||
        testBasicN3() != 0 ||
        testHFlipN3() != 0 ||
        testVFlipN3() != 0 ||
        testHVFlipN3() != 0 ||
        testVEdgeFixN3() != 0 ||
        testHEdgeFixN3() != 0 ||
        testHVEdgeFixN3() != 0 ||
        testHVEdgeFixOneSolution() != 0 ||
        testPattern() != 0 ||
        testHVEdgeFixPattern() != 0 ||
        testWide() != 0 ||
        testTall() != 0 ||
        testSrcBiggerThanDst() != 0 ||
        testPatternCountBasic() != 0 ||
        testPatternCountHFlip() != 0 ||
        testPatternCountVFlip() != 0 ||
        testPatternCountHVFlip() != 0 ||
        testPatternCountRotate() != 0 ||
        testPatternCountRotateCentralSymmetric() != 0 ||
        testPatternCountHFlipRotate() != 0 ||
        testPatternCountVFlipRotate() != 0 ||
        testPatternCountHVFlipRotate() != 0 ||
        testClone() != 0) {
        printf("Seed was: %u\n", seed);
        return 1;
    }

    return 0;
}
