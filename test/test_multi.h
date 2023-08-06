#ifndef INCLUDE_TEST_MULTI_H
#define INCLUDE_TEST_MULTI_H

#include "wfc.h"

int wfcGenerate(
    int n, int options, int bytesPerPixel,
    int srcW, int srcH, const unsigned char *src,
    int dstW, int dstH, unsigned char *dst);

#endif
