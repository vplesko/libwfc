// @TODO implement

int wfc_generatePixels(
    int bytesPerPixel,
    int srcW, int srcH, const unsigned char *src,
    int dstW, int dstH, unsigned char *dst) {
    (void)srcW;
    (void)srcH;
    (void)src;

    for (int i = 0; i < dstW * dstH * bytesPerPixel; ++i) {
        dst[i] = 0x7f;
    }

    return 0;
}
