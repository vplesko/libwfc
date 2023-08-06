#include <stdint.h>

#include "test_multi.h"

int main(void) {
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
        return 1;
    }

    for (int i = 0; i < dstW * dstH; ++i) {
        if (dst[i] != 5 && dst[i] != 6) {
            return 1;
        }
    }

    return 0;
}
