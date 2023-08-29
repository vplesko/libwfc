#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "stb_image.h"

#define WFC_IMPLEMENTATION
#include "wfc.h"

// @TODO After optimizing some more, replace GIFs in README with faster ones.

const int repeats = 5;

double measure(
    int n, int options, int bytesPerPixel,
    int srcW, int srcH, const unsigned char *src,
    int dstW, int dstH, unsigned char *dst) {
    clock_t t0 = clock();
    int res = wfc_generate(n, options, bytesPerPixel,
        srcW, srcH, src, dstW, dstH, dst);
    clock_t t1 = clock();

    assert(res == 0);

    return (double)(t1 - t0) / (double)CLOCKS_PER_SEC;
}

void benchmark(
    int n, int options, int bytesPerPixel,
    int srcW, int srcH, const unsigned char *src,
    int dstW, int dstH, unsigned char *dst) {
    bool first = true;
    double avg = 0.0, min = 0.0, max = 0.0;

    putchar('\t');
    for (int i = 0; i < repeats; ++i) {
        double elapsed = measure(n, options, bytesPerPixel,
            srcW, srcH, src, dstW, dstH, dst);

        avg += elapsed;
        if (first || elapsed < min) min = elapsed;
        if (first || elapsed > max) max = elapsed;
        first = false;

        if (i % 5 != 4) putchar('.');
        else putchar('o');
        fflush(stdout);
    }
    avg /= repeats;

    putchar('\n');
    printf("\tavg=%.4f min=%.4f max=%.4f\n", avg, min, max);
}

void benchmarkImage(const char *path, int n, int options, int dstW, int dstH) {
    const int bytesPerPixel = 4;

    unsigned char *src = NULL;
    unsigned char *dst = NULL;

    int srcW, srcH;
    src = stbi_load(path, &srcW, &srcH, NULL, bytesPerPixel);
    assert(src != NULL);

    dst = malloc(dstW * dstH * bytesPerPixel);

    printf("image=%s repeats=%d args={n=%d dstW=%d dstH=%d}\n",
        path, repeats, n, dstW, dstH);

    benchmark(n, options, bytesPerPixel, srcW, srcH, src, dstW, dstH, dst);

    free(dst);
    stbi_image_free(src);
}

void benchmarkText(const char *path, int n, int options, int dstW, int dstH) {
    FILE *file = NULL;
    uint32_t *src = NULL;
    uint32_t *dst = NULL;

    file = fopen(path, "r");
    assert(file != NULL);

    int srcW, srcH;
    assert(fscanf(file, "%d", &srcW) == 1);
    assert(fscanf(file, "%d", &srcH) == 1);

    src = malloc(srcW * srcH * sizeof(*src));
    for (int i = 0; i < srcW * srcH; ++i) {
        uint32_t u;
        assert(fscanf(file, "%u", &u) == 1);
        src[i] = u;
    }

    dst = malloc(dstW * dstH * sizeof(*dst));

    printf("image=%s repeats=%d args={n=%d dstW=%d dstH=%d}\n",
        path, repeats, n, dstW, dstH);

    benchmark(n, options, sizeof(*src),
        srcW, srcH, (unsigned char*)src, dstW, dstH, (unsigned char*)dst);

    free(dst);
    free(src);
    fclose(file);
}

int main(void) {
    srand((unsigned)time(NULL));

    benchmarkImage("external/samples/Angular.png", 3, 0, 64, 64);

    putchar('\n');
    benchmarkText("benchmark/test.txt", 5, 0, 120, 120);

    return 0;
}
