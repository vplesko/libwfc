#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "wfc.h"

#define REPEATS 5

double measure(
    int n,
    int srcW, int srcH, const uint32_t *src,
    int dstW, int dstH, uint32_t *dst) {
    clock_t t0 = clock();
    int res = wfc_generate(n, srcW, srcH, src, dstW, dstH, dst);
    clock_t t1 = clock();

    assert(res == 0);

    return (double)(t1 - t0) / (double)CLOCKS_PER_SEC;
}

void benchmark(
    int n,
    int srcW, int srcH, const uint32_t *src,
    int dstW, int dstH, uint32_t *dst) {
    int first = 1;
    double avg = 0.0, min, max;

    putchar('\t');
    for (int i = 0; i < REPEATS; ++i) {
        double elapsed = measure(n, srcW, srcH, src, dstW, dstH, dst);

        avg += elapsed;
        if (first || elapsed < min) min = elapsed;
        if (first || elapsed > max) max = elapsed;
        first = 0;

        if (i % 5 != 4) putchar('.');
        else putchar('o');
        fflush(stdout);
    }
    avg /= REPEATS;

    putchar('\n');
    printf("\tavg=%.4f min=%.4f max=%.4f\n", avg, min, max);
}

void benchmarkImage(const char *path, int n, int dstW, int dstH) {
    unsigned char *src = NULL;
    uint32_t *dst = NULL;

    int srcW, srcH;
    src = stbi_load(path, &srcW, &srcH, NULL, 4);
    assert(src != NULL);

    dst = malloc(dstW * dstH * sizeof(*dst));

    printf("image=%s repeats=%d args={n=%d dstW=%d dstH=%d}\n",
        path, REPEATS, n, dstW, dstH);

    benchmark(n, srcW, srcH, (uint32_t*)src, dstW, dstH, dst);

    free(dst);
    stbi_image_free(src);
}

void benchmarkText(const char *path, int n, int dstW, int dstH) {
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
        path, REPEATS, n, dstW, dstH);

    benchmark(n, srcW, srcH, src, dstW, dstH, dst);

    free(dst);
    free(src);
    fclose(file);
}

int main(void) {
    srand((unsigned)time(NULL));

    benchmarkImage("../external/samples/Angular.png", 3, 64, 64);

    putchar('\n');
    benchmarkText("test.txt", 5, 120, 120);

    return 0;
}
