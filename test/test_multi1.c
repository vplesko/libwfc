#include "test_multi.h"

#define WFC_IMPLEMENTATION
#include "wfc.h"

int wfcGenerate(
    int n, int options, int bytesPerPixel,
    int srcW, int srcH, const unsigned char *src,
    int dstW, int dstH, unsigned char *dst) {
    return wfc_generate(
        n, options, bytesPerPixel,
        srcW, srcH, src,
        dstW, dstH, dst);
}
