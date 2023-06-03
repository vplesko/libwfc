#include <stdint.h>
#include <stdio.h>

#include "tests.h"
#include "wfc.h"

int testBasicN1(void) {
    uint32_t src[4][4] = {
        {5,5,5,5},
        {5,5,6,5},
        {5,6,6,5},
        {5,5,5,5},
    };
    uint32_t dst[16][16];

    int res = wfc_generate(1, 4, 4, (uint32_t*)&src, 16, 16, (uint32_t*)&dst);
    if (res != 0) {
        PRINT_TEST_FAIL();
        return 1;
    }

    for (size_t x = 0; x < ARR_LEN(dst); ++x) {
        for (size_t y = 0; y < ARR_LEN(dst[x]); ++y) {
            if (dst[x][y] != 5 && dst[x][y] != 6) {
                PRINT_TEST_FAIL();
                return 1;
            }
        }
    }

    return 0;
}

int main(void) {
    int ret = 0;

    ret = testBasicN1();
    if (ret != 0) return ret;

    return ret;
}
