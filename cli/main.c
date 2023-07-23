#include <time.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "cmd_args.h"
#include "wfc_wrap.h"

void logError(const char *msg) {
    fprintf(stderr, "%s\n", msg);
}

int main(int argc, char *argv[]) {
    int ret = 0;

    const int bytesPerPixel = 4;

    unsigned char *srcPixels = NULL;
    unsigned char *dstPixels = NULL;

    const char *imagePath;
    int wfcN, dstW, dstH;
    if (parseArgs(argc, argv, &imagePath, &wfcN, &dstW, &dstH) != 0) {
        logError("Invalid arguments.");
        ret = 1;
        goto cleanup;
    }

    int srcW, srcH;
    srcPixels = stbi_load(imagePath, &srcW, &srcH, NULL, bytesPerPixel);
    if (srcPixels == NULL) {
        logError(stbi_failure_reason());
        ret = 1;
        goto cleanup;
    }

    srand((unsigned)time(NULL));

    dstPixels = malloc(dstW * dstH * bytesPerPixel);
    if (wfcGenerate(
            wfcN, 0, bytesPerPixel,
            srcW, srcH, srcPixels,
            dstW, dstH, dstPixels) != 0) {
        logError("WFC failed.");
        ret = 1;
        goto cleanup;
    }

    // dummy printout to make sure the compiler doesn't optimize anything away
    uint32_t dummy = 0;
    for (int i = 0; i < dstW * dstH * bytesPerPixel; ++i) {
        dummy += dstPixels[i];
    }
    printf("%u\n", dummy);

cleanup:
    free(dstPixels);
    stbi_image_free(srcPixels);

    return ret;
}
