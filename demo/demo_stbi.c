#include <stdlib.h>
#include <time.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "wfc.h"

void logError(const char *msg) {
    fprintf(stderr, "%s\n", msg);
}

int parseArgs(int argc, char *argv[],
    const char **imagePath, int *wfcN, int *dstW, int *dstH) {
    if (argc < 5) {
        logError("Invalid arguments.");
        return 1;
    }

    *imagePath = argv[1];

    long l;
    char *end;

    l = strtol(argv[2], &end, 0);
    if (*end != '\0') {
        logError("Invalid arguments.");
        return 1;
    }
    *wfcN = (int)l;

    l = strtol(argv[3], &end, 0);
    if (*end != '\0') {
        logError("Invalid arguments.");
        return 1;
    }
    *dstW = (int)l;

    l = strtol(argv[4], &end, 0);
    if (*end != '\0') {
        logError("Invalid arguments.");
        return 1;
    }
    *dstH = (int)l;

    return 0;
}

int main(int argc, char *argv[]) {
    int ret = 0;

    unsigned char *srcPixels = NULL;
    uint32_t *dstPixels = NULL;

    const char *imagePath;
    int wfcN, dstW, dstH;
    if (parseArgs(argc, argv, &imagePath, &wfcN, &dstW, &dstH) != 0) {
        ret = 1;
        goto cleanup;
    }

    int srcW, srcH;
    srcPixels = stbi_load(imagePath, &srcW, &srcH, NULL, 4);
    if (srcPixels == NULL) {
        logError(stbi_failure_reason());
        ret = 1;
        goto cleanup;
    }

    srand((unsigned)time(NULL));

    dstPixels = malloc(dstW * dstH * 4);
    // @TODO see if my new MDA macros made the code slower
    if (wfc_generate(
            wfcN,
            srcW, srcH, (uint32_t*)srcPixels,
            dstW, dstH, dstPixels) != 0) {
        logError("WFC failed.");
        ret = 1;
        goto cleanup;
    }

    // dummy printout to make sure the compiler doesn't optimize anything away
    uint32_t dummy = 0;
    for (int i = 0; i < dstW * dstH; ++i) {
        dummy += dstPixels[i];
    }
    printf("%u\n", dummy);

cleanup:
    free(dstPixels);
    stbi_image_free(srcPixels);

    return ret;
}
