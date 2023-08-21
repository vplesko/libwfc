#include "test_multi.h"

#include <assert.h>
#include <stdlib.h>

struct Context {
    int x;
};

void* customMalloc(void *ctx, size_t sz) {
    if (ctx != NULL) ++((struct Context*)ctx)->x;
    return malloc(sz);
}

void customFree(void *ctx, void *p) {
    if (ctx != NULL) ++((struct Context*)ctx)->x;
    free(p);
}

float customRand(void *ctx) {
    if (ctx != NULL) ++((struct Context*)ctx)->x;
    return (float)rand() / ((float)RAND_MAX + 1.0f);
}

// In addition to multi translation unit testing, this test also tests that
// overriding these macros works correctly.
#define WFC_ASSERT(ctx, cond) \
    do { \
        if (ctx != NULL) ++((struct Context*)ctx)->x; \
        assert(cond); \
    } while (false)

#define WFC_MALLOC(ctx, sz) customMalloc(ctx, sz)
#define WFC_FREE(ctx, p) customFree(ctx, p)
#define WFC_RAND(ctx) customRand(ctx)

#define WFC_IMPLEMENTATION
#include "wfc.h"

int wfcGenerate(
    int n, int options, int bytesPerPixel,
    int srcW, int srcH, const unsigned char *src,
    int dstW, int dstH, unsigned char *dst) {
    struct Context ctx;

    return wfc_generateEx(
        n, options, bytesPerPixel,
        srcW, srcH, src,
        dstW, dstH, dst,
        &ctx, NULL);
}
