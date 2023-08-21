/*
MIT License

Copyright (c) 2023 Vladimir Pleskonjic

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef INCLUDE_WFC_H
#define INCLUDE_WFC_H

#include <stdbool.h>

// @TODO Allow different values of N for different dimensions.
// @TODO Implement 3D WFC, with GUI support.

#ifdef __cplusplus
extern "C" {
#endif

enum {
    // Status code that signifies that WFC has successfully completed.
    wfc_completed = 1,
    // Status code that signifies that WFC has failed due to running into a
    // contradiction.
    wfc_failed = -1,
    // Status code that signifies that there was an error in provided arguments.
    wfc_callerError = -2
};

enum {
    // Enable this option to allow horizontal flipping of patterns (think of
    // y-axis as the mirror).
    wfc_optFlipH = 1 << 1,
    // Enable this option to allow vertical flipping of patterns (think of
    // x-axis as the mirror).
    wfc_optFlipV = 1 << 0,

    // Enable this option to allow rotating of patterns (by 90, 180, and 270
    // degrees).
    wfc_optRotate = 1 << 2,

    // Enable this option to fix left and right edges of input image so that
    // patterns may not wrap around them.
    wfc_optEdgeFixH = 1 << 4,
    // Enable this option to fix top and bottom edges of input image so that
    // patterns may not wrap around them.
    wfc_optEdgeFixV = 1 << 3
};

// An opaque struct containing the WFC state. You should only interact with it
// through a pointer.
typedef struct wfc_State wfc_State;

/**
 * Runs WFC on the provided source image and blits to the destination.
 *
 * \param n Pattern size will be \c n by \c n pixels. Must be positive and not
 * greater than any dimension of source and destination images.
 *
 * \param options Bitmask determining how WFC will run. This should be a
 * bitwise-or of wfc_opt* values or zero.
 *
 * \param bytesPerPixel Determines the byte size of a single pixel value in
 * source and destination images. These values will be compared with a simple
 * memcmp, so make sure that all unused bits are set to zero. Must be positive.
 *
 * \param srcW Width in pixels of the source image. Must be positive.
 *
 * \param srcH Height in pixels of the source image. Must be positive.
 *
 * \param src Pointer to a row-major array of pixels comprising the source
 * image. Must not be null.
 *
 * \param dstW Width in pixels of the destination image. Must be positive.
 *
 * \param dstH Height in pixels of the destination image. Must be positive.
 *
 * \param dst Pointer to a row-major array of pixels comprising the destination
 * image. WFC output will be blitted here.
 *
 * \return Returns the status code of WFC, which is one of:
 *
 * \li wfc_completed (positive) in case of success;
 * \li wfc_failed (negative) in case of contradiction;
 * \li wfc_callerError (negative) in case of argument error.
 *
 * On success, the generated image will be written to \c dst.
 */
int wfc_generate(
    int n, int options, int bytesPerPixel,
    int srcW, int srcH, const unsigned char *src,
    int dstW, int dstH, unsigned char *dst);

/**
 * Runs WFC on the provided source image and blits to the destination. Same as
 * wfc_generate except that it provides more capabilities.
 *
 * \param n Pattern size will be \c n by \c n pixels. Must be positive and not
 * greater than any dimension of source and destination images.
 *
 * \param options Bitmask determining how WFC will run. This should be a
 * bitwise-or of wfc_opt* values or zero.
 *
 * \param bytesPerPixel Determines the size in bytes of a single value in source
 * and destination images. These values will be compared with a simple memcmp,
 * so make sure that all unused bits are set to zero. Must be positive.
 *
 * \param srcW Width in pixels of the source image. Must be positive.
 *
 * \param srcH Height in pixels of the source image. Must be positive.
 *
 * \param src Pointer to a row-major array of pixels comprising the source
 * image. Must not be null.
 *
 * \param dstW Width in pixels of the destination image. Must be positive.
 *
 * \param dstH Height in pixels of the destination image. Must be positive.
 *
 * \param dst Pointer to a row-major array of pixels comprising the destination
 * image. WFC output will be blitted here. If \c keep is non-null then this must
 * not be null.
 *
 * \param ctx User context that will be passed to WFC_ASSERT, WFC_MALLOC,
 * WFC_FREE, and WFC_RAND.
 *
 * \param keep If non-null, signifies that some values in \c dst are
 * pre-determined and that WFC should not modify them. WFC will attempt to
 * generate the rest of the output image, while keeping these values as they
 * are. If non-null, must be of the same dimensions as \c dst - true means keep
 * that pixel value unchanged.
 *
 * \return Returns the status code of WFC, which is one of:
 *
 * \li wfc_completed (positive) in case of success;
 * \li wfc_failed (negative) in case of contradiction;
 * \li wfc_callerError (negative) in case of argument error.
 *
 * On success, the generated image will be written to \c dst.
 */
int wfc_generateEx(
    int n, int options, int bytesPerPixel,
    int srcW, int srcH, const unsigned char *src,
    int dstW, int dstH, unsigned char *dst,
    void *ctx,
    bool *keep);

/**
 * Allocates and initializes a state object for WFC. This is a first step
 * towards running WFC, you will likely be using wfc_step after.
 *
 * Consider using wfc_generate instead if you don't need any custom handling of
 * individual steps.
 *
 * \param n Pattern size will be \c n by \c n pixels. Must be positive and not
 * greater than any dimension of source and destination images.
 *
 * \param options Bitmask determining how WFC will run. This should be a
 * bitwise-or of wfc_opt* values or zero.
 *
 * \param bytesPerPixel Determines the size in bytes of a single value in source
 * and destination images. These values will be compared with a simple memcmp,
 * so make sure that all unused bits are set to zero. Must be positive.
 *
 * \param srcW Width in pixels of the source image. Must be positive.
 *
 * \param srcH Height in pixels of the source image. Must be positive.
 *
 * \param src Pointer to a row-major array of pixels comprising the source
 * image. Must not be null.
 *
 * \param dstW Width in pixels of the destination image. Must be positive.
 *
 * \param dstH Height in pixels of the destination image. Must be positive.
 *
 * \return Returns an allocated state object to be passed to further WFC
 * functions. This object should be deallocated using wfc_free.
 *
 * In case of error, returns null.
 */
wfc_State* wfc_init(
    int n, int options, int bytesPerPixel,
    int srcW, int srcH, const unsigned char *src,
    int dstW, int dstH);

/**
 * Allocates and initializes a state object for WFC. This is a first step
 * towards running WFC, you will likely be using wfc_step after. Same as
 * wfc_init except that it provides more capabilities.
 *
 * Consider using wfc_generate instead if you don't need any custom handling of
 * individual steps.
 *
 * \param n Pattern size will be \c n by \c n pixels. Must be positive and not
 * greater than any dimension of source and destination images.
 *
 * \param options Bitmask determining how WFC will run. This should be a
 * bitwise-or of wfc_opt* values or zero.
 *
 * \param bytesPerPixel Determines the size in bytes of a single value in source
 * and destination images. These values will be compared with a simple memcmp,
 * so make sure that all unused bits are set to zero. Must be positive.
 *
 * \param srcW Width in pixels of the source image. Must be positive.
 *
 * \param srcH Height in pixels of the source image. Must be positive.
 *
 * \param src Pointer to a row-major array of pixels comprising the source
 * image. Must not be null.
 *
 * \param dstW Width in pixels of the destination image. Must be positive.
 *
 * \param dstH Height in pixels of the destination image. Must be positive.
 *
 * \param dst Pointer to a row-major array of pixels comprising the destination
 * image. During initialization, destination image is only used for the sake of
 * images to be kept (see the \c keep parameter). If \c keep is non-null then
 * this must not be null.
 *
 * \param ctx User context that will be passed to WFC_ASSERT, WFC_MALLOC,
 * WFC_FREE, and WFC_RAND.
 *
 * \param keep If non-null, signifies that some values in \c dst are
 * pre-determined and that WFC should not modify them. WFC will attempt to
 * generate the rest of the output image, while keeping these values as they
 * are. If non-null, must be of the same dimensions as \c dst - true means keep
 * that pixel value unchanged.
 *
 * \return Returns an allocated state object to be passed to further WFC
 * functions. This object should be deallocated using wfc_free.
 *
 * In case of error, returns null.
 */
wfc_State* wfc_initEx(
    int n, int options, int bytesPerPixel,
    int srcW, int srcH, const unsigned char *src,
    int dstW, int dstH, const unsigned char *dst,
    void *ctx,
    bool *keep);

/**
 * Returns the current status code for this WFC state.
 *
 * \param state State object pointer for which to query status. Must not be
 * null.
 *
 * \return Returns the current status code, which is one of:
 *
 * \li 0 (zero) in case that WFC has not completed yet (you can keep calling
 * wfc_step);
 * \li wfc_completed (positive) in case that WFC has completed successfully;
 * \li wfc_failed (negative) in case that WFC has reached a contradiction and
 * failed to complete;
 * \li wfc_callerError (negative) in case \c state was null.
*/
int wfc_status(const wfc_State *state);

/**
 * Performs one iteration of the WFC algorithm, observing one wave point and
 * propagating constraints. After WFC completes, you will likely be calling
 * wfc_blit next.
 *
 * \param state State object pointer on which to perform the iteration. Must not
 * be null.
 *
 * \return Returns the status code after calling wfc_step, which is one of:
 *
 * \li 0 (zero) in case that WFC has not completed yet (you can keep calling
 * wfc_step);
 * \li wfc_completed (positive) in case that WFC has completed successfully;
 * \li wfc_failed (negative) in case that WFC has reached a contradiction and
 * failed to complete;
 * \li wfc_callerError (negative) in case \c state was null.
*/
int wfc_step(wfc_State *state);

/**
 * Blits (aka. renders) the generated image to \c dst by copying in the pixel
 * values. Should be called after WFC completes successfully (after wfc_step has
 * returned wfc_completed).
 *
 * \param state State object pointer. Must not be null. Must be in the completed
 * state.
 *
 * \param src Pointer to pixels comprising the source image. Must not be null.
 * Must be of the same dimensions as the image that was passed to wfc_init.
 *
 * \param dst Pointer to pixels comprising the destination image. Must not be
 * null. Must be of the dimensions that were specified in wfc_init.
 *
 * \return Returns zero on success or wfc_callerError if there was an error in
 * the arguments.
*/
int wfc_blit(
    const wfc_State *state,
    const unsigned char *src, unsigned char *dst);

/**
 * Allocates a new state object as a deep-copy of the provided one. The new
 * object is completely independent of the old one - you can call wfc_step on it
 * and both objects need to be deallocated with wfc_free.
 *
 * \param state Pointer to the state object to be cloned.
 *
 * \return Returns a new state object that is a copy of the provided one. If
 * \c state is null, null is returned instead.
*/
wfc_State* wfc_clone(const wfc_State *state);

/**
 * Deallocates the state object and all data owned by it. The state pointer
 * should not be used after this function is called.
 *
 * \param state Pointer to the state object to deallocate.
*/
void wfc_free(wfc_State *state);

/**
 * Returns the number of unique patterns gathered from the input image when the
 * provided state object was being initialized.
 *
 * \param state State object pointer for which to query the number of unique
 * patterns. Must not be null.
 *
 * \return Returns the number of unique patterns gathered from the input image.
 * Returns wfc_callerError (a negative value) if \c state is null.
*/
int wfc_patternCount(const wfc_State *state);

/**
 * Returns whether a particular pattern is still present at the wave point with
 * the given coordinates. As WFC iterates, patterns from wave points get removed
 * as points are observed and constraints are propagated - this function lets
 * you know which patterns are still left at specific points.
 *
 * When calling this, you need to provide a valid pattern index. Valid pattern
 * indexes are in the range from zero (inclusive) to pattern count (exclusive).
 * You can get the pattern count by calling wfc_patternCount.
 *
 * \param state State object pointer containing the wave to be queried. Must not
 * be null.
 *
 * \param patt Pattern index whose presence is being queried. Must be a valid
 * index.
 *
 * \param x x coordinate of the destination image. Must be a valid x coordinate
 * for the image being generated.
 *
 * \param y y coordinate of the destination image. Must be a valid y coordinate
 * for the image being generated.
 *
 * \return Returns:
 *
 * \li 0 if the pattern is not present at that wave point;
 * \li 1 if the pattern is present at that wave point;
 * \li wfc_callerError (a negative value) if there was an error in the
 * arguments.
*/
int wfc_patternAvailable(const wfc_State *state, int patt, int x, int y);

/**
 * Returns a pointer to the bytes of the pixel value that would be blitted to a
 * destination image at the given coordinates if the given pattern was the one
 * chosen for the corresponding wave point.
 *
 * In other words, since WFC may rotate/flip/etc. the patterns it gathers (and
 * pattern order is not guaranteed), this function tells you what pixel would
 * end up at a destination image pixel for a specific pattern.
 *
 * When calling this, you need to provide a valid pattern index. Valid pattern
 * indexes are in the range from zero (inclusive) to pattern count (exclusive).
 * You can get the pattern count by calling wfc_patternCount.
 *
 * \param state State object pointer containing the wave to be queried. Must not
 * be null.
 *
 * \param patt Pattern index whose presence is being queried. Must be a valid
 * index.
 *
 * \param x x coordinate of the destination image. Must be a valid x coordinate
 * for the image being generated.
 *
 * \param y y coordinate of the destination image. Must be a valid y coordinate
 * for the image being generated.
 *
 * \param src Pointer to pixels comprising the source image. Must not be null.
 * Must be of the same dimensions as the image that was passed to wfc_init.
 *
 * \return Returns a pointer to the bytes for the corresponding pixel value.
 * Returns null if there was an error in the arguments.
*/
const unsigned char* wfc_pixelToBlit(
    const wfc_State *state,
    int patt, int x, int y, const unsigned char *src);

#ifdef __cplusplus
}
#endif

#endif // INCLUDE_WFC_H

// IMPLEMENTATION

#ifdef WFC_IMPLEMENTATION

#include <float.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef WFC_ASSERT
#include <assert.h>
#define WFC_ASSERT(ctx, cond) assert(cond)
#endif

#ifndef WFC_MALLOC
#define WFC_MALLOC(ctx, sz) malloc(sz)
#endif

#ifndef WFC_FREE
#define WFC_FREE(ctx, p) free(p)
#endif

// This macro should return a float in [0, 1).
#ifndef WFC_RAND
#define WFC_RAND(ctx) wfc__rand()
#endif

// basic utility

int wfc__min_i(int a, int b) {
    return a < b ? a : b;
}

int wfc__max_i(int a, int b) {
    return a < b ? b : a;
}

bool wfc__approxEq_f(float a, float b) {
    const float absDiff = 0.001f;
    const float relDiff = FLT_EPSILON;

    if (fabsf(a - b) < absDiff) return true;

    if (fabsf(a) < fabsf(b)) return fabsf((a - b) / b) < relDiff;
    return fabsf((a - b) / a) < relDiff;
}

// RNG utility

// [0, 1)
float wfc__rand(void) {
    return (float)rand() / ((float)RAND_MAX + 1.0f);
}

// [0, n)
int wfc__rand_i(void *ctx, int n) {
    (void)ctx;
    return (int)(WFC_RAND(ctx) * (float)n);
}

// multi-dimensional array utility

int wfc__indWrap(int ind, int sz) {
    if (ind >= 0) return ind % sz;
    return sz + ind % sz;
}

#define WFC__A2D_DEF(type, abbrv) \
    struct wfc__A2d_##abbrv { \
        int d02, d12; \
        type *a; \
    }

#define WFC__A2D_LEN(arr) ((arr).d02 * (arr).d12)

#define WFC__A2D_SIZE(arr) ((size_t)WFC__A2D_LEN(arr) * sizeof(*(arr).a))

#define WFC__A2D_GET(arr, c0, c1) \
    ((arr).a[ \
        (c0) * (arr).d12 + \
        (c1)])

#define WFC__A2D_GET_WRAP(arr, c0, c1) \
    ((arr).a[ \
        wfc__indWrap(c0, (arr).d02) * (arr).d12 + \
        wfc__indWrap(c1, (arr).d12)])

#define WFC__A3D_DEF(type, abbrv) \
    struct wfc__A3d_##abbrv { \
        int d03, d13, d23; \
        type *a; \
    }

#define WFC__A3D_LEN(arr) ((arr).d03 * (arr).d13 * (arr).d23)

#define WFC__A3D_SIZE(arr) ((size_t)WFC__A3D_LEN(arr) * sizeof(*(arr).a))

#define WFC__A3D_GET(arr, c0, c1, c2) \
    ((arr).a[ \
        (c0) * (arr).d13 * (arr).d23 + \
        (c1) * (arr).d23 + \
        (c2)])

#define WFC__A3D_GET_WRAP(arr, c0, c1, c2) \
    ((arr).a[ \
        wfc__indWrap(c0, (arr).d03) * (arr).d13 * (arr).d23 + \
        wfc__indWrap(c1, (arr).d13) * (arr).d23 + \
        wfc__indWrap(c2, (arr).d23)])

#define WFC__A4D_DEF(type, abbrv) \
    struct wfc__A4d_##abbrv { \
        int d04, d14, d24, d34; \
        type *a; \
    }

#define WFC__A4D_LEN(arr) ((arr).d04 * (arr).d14 * (arr).d24 * (arr).d34)

#define WFC__A4D_SIZE(arr) ((size_t)WFC__A4D_LEN(arr) * sizeof(*(arr).a))

#define WFC__A4D_GET(arr, c0, c1, c2, c3) \
    ((arr).a[ \
        (c0) * (arr).d14 * (arr).d24 * (arr).d34 + \
        (c1) * (arr).d24 * (arr).d34 + \
        (c2) * (arr).d34 + \
        (c3)])

#define WFC__A4D_GET_WRAP(arr, c0, c1, c2, c3) \
    ((arr).a[ \
        wfc__indWrap(c0, (arr).d04) * (arr).d14 * (arr).d24 * (arr).d34 + \
        wfc__indWrap(c1, (arr).d14) * (arr).d24 * (arr).d34 + \
        wfc__indWrap(c2, (arr).d24) * (arr).d34 + \
        wfc__indWrap(c3, (arr).d34)])

void wfc__indToCoords2d(int d1, int ind, int *c0, int *c1) {
    int c0_ = ind / d1;
    ind -= c0_ * d1;
    int c1_ = ind;

    *c0 = c0_;
    *c1 = c1_;
}

// Bools are not used in some places due to performance concerns.
WFC__A2D_DEF(bool, b);
WFC__A2D_DEF(uint8_t, u8);
WFC__A2D_DEF(float, f);
WFC__A3D_DEF(uint8_t, u8);
WFC__A3D_DEF(const uint8_t, cu8);
WFC__A4D_DEF(uint8_t, u8);

// WFC code

enum {
    wfc__optFlipC0 = wfc_optFlipV,
    wfc__optFlipC1 = wfc_optFlipH,
    wfc__optEdgeFixC0 = wfc_optEdgeFixV,
    wfc__optEdgeFixC1 = wfc_optEdgeFixH
};

// @TODO Some option combinations are equivalent, so work in gathering patterns
// gets duplicated. Optimize by removing that work duplication.
enum {
    wfc__tfFlipC0 = 1 << 0,
    wfc__tfFlipC1 = 1 << 1,
    wfc__tfRot90 = 1 << 2,
    wfc__tfRot180 = 1 << 3,
    wfc__tfRot270 = wfc__tfRot90 | wfc__tfRot180,

    wfc__tfCnt = 1 << 4
};

struct wfc__Pattern {
    int c0, c1;
    int tf;

    int edgeC0Lo, edgeC0Hi, edgeC1Lo, edgeC1Hi;
    int freq;
};

void wfc__coordsPattToSrc(
    int n,
    struct wfc__Pattern patt, int pC0, int pC1,
    int sD0, int sD1, int *sC0, int *sC1) {
    int tfC0 = pC0;
    int tfC1 = pC1;

    // map from patt space to src space
    {
        if (patt.tf & wfc__tfFlipC0) tfC0 = n - 1 - tfC0;
        if (patt.tf & wfc__tfFlipC1) tfC1 = n - 1 - tfC1;

        // rot270 is rot90 plus rot180 (both in bitmask and as transformation)
        if (patt.tf & wfc__tfRot90) {
            int tmpC0 = tfC0;
            int tmpC1 = tfC1;

            tfC0 = tmpC1;
            tfC1 = n - 1 - tmpC0;
        }
        if (patt.tf & wfc__tfRot180) {
            tfC0 = n - 1 - tfC0;
            tfC1 = n - 1 - tfC1;
        }
    }

    int sC0_ = wfc__indWrap(patt.c0 + tfC0, sD0);
    int sC1_ = wfc__indWrap(patt.c1 + tfC1, sD1);

    if (sC0 != NULL) *sC0 = sC0_;
    if (sC1 != NULL) *sC1 = sC1_;
}

void wfc__fillPattEdges(int n, int sD0, int sD1,
    struct wfc__Pattern *patt) {
    bool edgeC0Lo = patt->c0 == 0;
    bool edgeC0Hi = patt->c0 + n == sD0;
    bool edgeC1Lo = patt->c1 == 0;
    bool edgeC1Hi = patt->c1 + n == sD1;

    // map from src space to patt space
    {
        // rot270 is rot90 plus rot180 (both in bitmask and as transformation)
        if (patt->tf & wfc__tfRot180) {
            bool tmpC0Lo = edgeC0Lo;
            bool tmpC0Hi = edgeC0Hi;
            bool tmpC1Lo = edgeC1Lo;
            bool tmpC1Hi = edgeC1Hi;

            edgeC0Lo = tmpC0Hi;
            edgeC0Hi = tmpC0Lo;
            edgeC1Lo = tmpC1Hi;
            edgeC1Hi = tmpC1Lo;
        }
        if (patt->tf & wfc__tfRot90) {
            bool tmpC0Lo = edgeC0Lo;
            bool tmpC0Hi = edgeC0Hi;
            bool tmpC1Lo = edgeC1Lo;
            bool tmpC1Hi = edgeC1Hi;

            edgeC0Lo = tmpC1Hi;
            edgeC0Hi = tmpC1Lo;
            edgeC1Lo = tmpC0Lo;
            edgeC1Hi = tmpC0Hi;
        }

        if (patt->tf & wfc__tfFlipC1) {
            bool tmp = edgeC1Lo;
            edgeC1Lo = edgeC1Hi;
            edgeC1Hi = tmp;
        }
        if (patt->tf & wfc__tfFlipC0) {
            bool tmp = edgeC0Lo;
            edgeC0Lo = edgeC0Hi;
            edgeC0Hi = tmp;
        }
    }

    patt->edgeC0Lo = edgeC0Lo;
    patt->edgeC0Hi = edgeC0Hi;
    patt->edgeC1Lo = edgeC1Lo;
    patt->edgeC1Hi = edgeC1Hi;
}

void wfc__coordsDstToWave(
    int dC0, int dC1,
    const struct wfc__A3d_u8 wave,
    int *wC0, int *wC1,
    int *offC0, int *offC1) {
    int wC0_ = wfc__min_i(dC0, wave.d03 - 1);
    int wC1_ = wfc__min_i(dC1, wave.d13 - 1);

    int offC0_ = dC0 - wC0_;
    int offC1_ = dC1 - wC1_;

    if (wC0 != NULL) *wC0 = wC0_;
    if (wC1 != NULL) *wC1 = wC1_;
    if (offC0 != NULL) *offC0 = offC0_;
    if (offC1 != NULL) *offC1 = offC1_;
}

int wfc__pattCombCnt(int d0, int d1) {
    return d0 * d1 * wfc__tfCnt;
}

void wfc__indToPattComb(int d1, int ind, struct wfc__Pattern *patt) {
    wfc__indToCoords2d(d1, ind / wfc__tfCnt, &patt->c0, &patt->c1);
    patt->tf = ind % wfc__tfCnt;
}

bool wfc__satisfiesOptions(int n, int options, int sD0, int sD1,
    struct wfc__Pattern patt) {
    if ((patt.tf & wfc__tfFlipC0) && !(options & wfc__optFlipC0)) return false;
    if ((patt.tf & wfc__tfFlipC1) && !(options & wfc__optFlipC1)) return false;

    if ((patt.tf & (wfc__tfRot90 | wfc__tfRot180 | wfc__tfRot270)) &&
        !(options & wfc_optRotate)) {
        return false;
    }

    if ((options & wfc__optEdgeFixC0) && patt.c0 + n > sD0) return false;
    if ((options & wfc__optEdgeFixC1) && patt.c1 + n > sD1) return false;

    return true;
}

bool wfc__patternsEq(
    int n, const struct wfc__A3d_cu8 src,
    struct wfc__Pattern pattA, struct wfc__Pattern pattB) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            int sAC0, sAC1;
            wfc__coordsPattToSrc(n, pattA, i, j, src.d03, src.d13,
                &sAC0, &sAC1);

            int sBC0, sBC1;
            wfc__coordsPattToSrc(n, pattB, i, j, src.d03, src.d13,
                &sBC0, &sBC1);

            const uint8_t *pxA = &WFC__A3D_GET(src, sAC0, sAC1, 0);
            const uint8_t *pxB = &WFC__A3D_GET(src, sBC0, sBC1, 0);

            if (memcmp(pxA, pxB, (size_t)src.d23) != 0) return false;
        }
    }

    return true;
}

struct wfc__Pattern* wfc__gatherPatterns(
    void *ctx,
    int n, int options,
    const struct wfc__A3d_cu8 src,
    int *cnt) {
    (void)ctx;

    struct wfc__Pattern *patts = NULL;

    int pattCnt = 0;
    for (int i = 0; i < wfc__pattCombCnt(src.d03, src.d13); ++i) {
        struct wfc__Pattern patt = {0, 0, 0, 0, 0, 0, 0, 0};
        wfc__indToPattComb(src.d13, i, &patt);
        if (!wfc__satisfiesOptions(n, options, src.d03, src.d13, patt)) {
            continue;
        }

        bool seenBefore = false;
        for (int i1 = 0; !seenBefore && i1 < i; ++i1) {
            struct wfc__Pattern patt1 = {0, 0, 0, 0, 0, 0, 0, 0};
            wfc__indToPattComb(src.d13, i1, &patt1);
            if (!wfc__satisfiesOptions(n, options, src.d03, src.d13, patt1)) {
                continue;
            }

            if (wfc__patternsEq(n, src, patt, patt1)) seenBefore = true;
        }

        if (!seenBefore) ++pattCnt;
    }

    patts = (struct wfc__Pattern*)WFC_MALLOC(
        ctx, (size_t)pattCnt * sizeof(*patts));
    int pattInd = 0;
    for (int i = 0; i < wfc__pattCombCnt(src.d03, src.d13); ++i) {
        struct wfc__Pattern patt = {0, 0, 0, 0, 0, 0, 0, 0};
        wfc__indToPattComb(src.d13, i, &patt);
        if (!wfc__satisfiesOptions(n, options, src.d03, src.d13, patt)) {
            continue;
        }

        wfc__fillPattEdges(n, src.d03, src.d13, &patt);
        patt.freq = 1;

        bool seenBefore = false;
        for (int i1 = 0; !seenBefore && i1 < pattInd; ++i1) {
            if (wfc__patternsEq(n, src, patt, patts[i1])) {
                patts[i1].edgeC0Lo |= patt.edgeC0Lo;
                patts[i1].edgeC0Hi |= patt.edgeC0Hi;
                patts[i1].edgeC1Lo |= patt.edgeC1Lo;
                patts[i1].edgeC1Hi |= patt.edgeC1Hi;

                ++patts[i1].freq;

                seenBefore = true;
            }
        }

        if (!seenBefore) patts[pattInd++] = patt;
    }
    WFC_ASSERT(ctx, pattInd == pattCnt);

    *cnt = pattCnt;
    return patts;
}

bool wfc__overlapMatches(
    void *ctx,
    int n, const struct wfc__A3d_cu8 src,
    int dC0, int dC1,
    struct wfc__Pattern pattA, struct wfc__Pattern pattB) {
    (void)ctx;

    WFC_ASSERT(ctx, abs(dC0) < n);
    WFC_ASSERT(ctx, abs(dC1) < n);

    int overlapD0 = n - abs(dC0);
    int overlapD1 = n - abs(dC1);

    int overlapPAC0 = dC0 > 0 ? dC0 : 0;
    int overlapPAC1 = dC1 > 0 ? dC1 : 0;
    int overlapPBC0 = dC0 > 0 ? 0 : abs(dC0);
    int overlapPBC1 = dC1 > 0 ? 0 : abs(dC1);

    for (int i = 0; i < overlapD0; ++i) {
        for (int j = 0; j < overlapD1; ++j) {
            int pAC0 = overlapPAC0 + i;
            int pAC1 = overlapPAC1 + j;

            int sAC0, sAC1;
            wfc__coordsPattToSrc(n, pattA, pAC0, pAC1, src.d03, src.d13,
                &sAC0, &sAC1);

            int pBC0 = overlapPBC0 + i;
            int pBC1 = overlapPBC1 + j;

            int sBC0, sBC1;
            wfc__coordsPattToSrc(n, pattB, pBC0, pBC1, src.d03, src.d13,
                &sBC0, &sBC1);

            const uint8_t *pxA = &WFC__A3D_GET(src, sAC0, sAC1, 0);
            const uint8_t *pxB = &WFC__A3D_GET(src, sBC0, sBC1, 0);

            if (memcmp(pxA, pxB, (size_t)src.d23) != 0) return false;
        }
    }

    return true;
}

struct wfc__A4d_u8 wfc__calcOverlaps(
    void *ctx,
    int n, const struct wfc__A3d_cu8 src,
    int pattCnt, const struct wfc__Pattern *patts) {
    struct wfc__A4d_u8 overlaps;
    overlaps.d04 = 2 * n - 1;
    overlaps.d14 = 2 * n - 1;
    overlaps.d24 = pattCnt;
    overlaps.d34 = pattCnt;
    overlaps.a = (uint8_t*)WFC_MALLOC(ctx, WFC__A4D_SIZE(overlaps));

    for (int dC0 = -(n - 1); dC0 <= n - 1; ++dC0) {
        for (int dC1 = -(n - 1); dC1 <= n - 1; ++dC1) {
            for (int i = 0; i < pattCnt; ++i) {
                for (int j = 0; j < pattCnt; ++j) {
                    bool overlap = wfc__overlapMatches(ctx, n, src,
                        dC0, dC1, patts[i], patts[j]);
                    WFC__A4D_GET(overlaps, dC0 + n - 1, dC1 + n - 1, i, j) =
                        overlap ? 1 : 0;
                }
            }
        }
    }

    return overlaps;
}

bool wfc__restrictKept(
    int n,
    const struct wfc__A3d_cu8 src,
    const struct wfc__Pattern *patts,
    const struct wfc__A3d_cu8 dst,
    const struct wfc__A2d_b keep,
    struct wfc__A3d_u8 wave) {
    const int pattCnt = wave.d23, bytesPerPixel = dst.d23;

    bool modified = false;

    for (int wC0 = 0; wC0 < wave.d03; ++wC0) {
        for (int wC1 = 0; wC1 < wave.d13; ++wC1) {
            for (int i = 0; i < n; ++i) {
                for (int j = 0; j < n; ++j) {
                    int dC0 = wfc__indWrap(wC0 + i, dst.d03);
                    int dC1 = wfc__indWrap(wC1 + j, dst.d13);

                    if (!WFC__A2D_GET(keep, dC0, dC1)) continue;

                    const unsigned char *dPx = &WFC__A3D_GET(dst, dC0, dC1, 0);

                    for (int p = 0; p < pattCnt; ++p) {
                        if (!WFC__A3D_GET(wave, wC0, wC1, p)) continue;

                        int sC0, sC1;
                        wfc__coordsPattToSrc(
                            n,
                            patts[p], i, j,
                            src.d03, src.d13,
                            &sC0, &sC1);

                        const unsigned char *sPx =
                            &WFC__A3D_GET(src, sC0, sC1, 0);

                        if (memcmp(dPx, sPx, (size_t)bytesPerPixel) != 0) {
                            WFC__A3D_GET(wave, wC0, wC1, p) = 0;
                            modified = true;
                        }
                    }
                }
            }
        }
    }

    return modified;
}

bool wfc__restrictEdges(
    int options,
    const struct wfc__Pattern *patts,
    struct wfc__A3d_u8 wave) {
    const int d0 = wave.d03, d1 = wave.d13, pattCnt = wave.d23;

    bool modified = false;

    if (options & wfc__optEdgeFixC0) {
        for (int i = 0; i < d1; ++i) {
            for (int p = 0; p < pattCnt; ++p) {
                if (WFC__A3D_GET(wave, 0, i, p) && !patts[p].edgeC0Lo) {
                    WFC__A3D_GET(wave, 0, i, p) = 0;
                    modified = true;
                }
                if (WFC__A3D_GET(wave, d0 - 1, i, p) && !patts[p].edgeC0Hi) {
                    WFC__A3D_GET(wave, d0 - 1, i, p) = 0;
                    modified = true;
                }
            }
        }
    }
    if (options & wfc__optEdgeFixC1) {
        for (int i = 0; i < d0; ++i) {
            for (int p = 0; p < pattCnt; ++p) {
                if (WFC__A3D_GET(wave, i, 0, p) && !patts[p].edgeC1Lo) {
                    WFC__A3D_GET(wave, i, 0, p) = 0;
                    modified = true;
                }
                if (WFC__A3D_GET(wave, i, d1 - 1, p) && !patts[p].edgeC1Hi) {
                    WFC__A3D_GET(wave, i, d1 - 1, p) = 0;
                    modified = true;
                }
            }
        }
    }

    return modified;
}

void wfc__calcEntropies(
    const struct wfc__Pattern *patts,
    const struct wfc__A3d_u8 wave,
    struct wfc__A2d_f entropies) {
    for (int c0 = 0; c0 < wave.d03; ++c0) {
        for (int c1 = 0; c1 < wave.d13; ++c1) {
            int totalFreq = 0;
            int availPatts = 0;
            for (int p = 0; p < wave.d23; ++p) {
                if (WFC__A3D_GET(wave, c0, c1, p)) {
                    totalFreq += patts[p].freq;
                    ++availPatts;
                }
            }

            float entropy = 0.0f;
            // check is here to ensure entropy of observed points becomes 0
            if (availPatts > 1) {
                for (int p = 0; p < wave.d23; ++p) {
                    if (WFC__A3D_GET(wave, c0, c1, p)) {
                        float prob = (float)patts[p].freq / (float)totalFreq;
                        entropy -= prob * log2f(prob);
                    }
                }
            }

            WFC__A2D_GET(entropies, c0, c1) = entropy;
        }
    }
}

void wfc__observeOne(
    void *ctx,
    int pattCnt, const struct wfc__Pattern *patts,
    const struct wfc__A2d_f entropies,
    struct wfc__A3d_u8 wave, int *obsC0, int *obsC1) {
    float smallest = 0;
    int smallestCnt = 0;
    for (int i = 0; i < WFC__A2D_LEN(entropies); ++i) {
        // skip observed points
        if (entropies.a[i] == 0.0f) continue;

        if (smallestCnt > 0 && wfc__approxEq_f(entropies.a[i], smallest)) {
            ++smallestCnt;
        } else if (smallestCnt == 0 || entropies.a[i] < smallest) {
            smallest = entropies.a[i];
            smallestCnt = 1;
        }
    }

    int chosenC0, chosenC1;
    {
        int chosenPnt = 0;
        int chosenSmallestPnt = wfc__rand_i(ctx, smallestCnt);
        for (int i = 0; i < WFC__A2D_LEN(entropies); ++i) {
            if (wfc__approxEq_f(entropies.a[i], smallest)) {
                chosenPnt = i;
                if (chosenSmallestPnt == 0) break;
                --chosenSmallestPnt;
            }
        }
        wfc__indToCoords2d(entropies.d12, chosenPnt, &chosenC0, &chosenC1);
    }

    int chosenPatt = 0;
    {
        int totalFreq = 0;
        for (int i = 0; i < pattCnt; ++i) {
            if (WFC__A3D_GET(wave, chosenC0, chosenC1, i)) {
                totalFreq += patts[i].freq;
            }
        }
        int chosenInst = wfc__rand_i(ctx, totalFreq);

        for (int i = 0; i < pattCnt; ++i) {
            if (WFC__A3D_GET(wave, chosenC0, chosenC1, i)) {
                if (chosenInst < patts[i].freq) {
                    chosenPatt = i;
                    break;
                }
                chosenInst -= patts[i].freq;
            }
        }
    }

    *obsC0 = chosenC0;
    *obsC1 = chosenC1;
    for (int i = 0; i < pattCnt; ++i) {
        WFC__A3D_GET(wave, chosenC0, chosenC1, i) = 0;
    }
    WFC__A3D_GET(wave, chosenC0, chosenC1, chosenPatt) = 1;
}

bool wfc__propagateOntoDelta(
    int n, int nC0, int nC1, int dC0, int dC1,
    const struct wfc__A4d_u8 overlaps,
    struct wfc__A3d_u8 wave) {
    const int c0 = wfc__indWrap(nC0 + dC0, wave.d03);
    const int c1 = wfc__indWrap(nC1 + dC1, wave.d13);
    const int pattCnt = wave.d23;

    int oldAvailPattCnt = 0;
    for (int p = 0; p < pattCnt; ++p) {
        if (WFC__A3D_GET(wave, c0, c1, p)) ++oldAvailPattCnt;
    }

    for (int p = 0; p < pattCnt; ++p) {
        if (!WFC__A3D_GET(wave, c0, c1, p)) continue;

        uint8_t total = 0;
        for (int pN = 0; pN < pattCnt; ++pN) {
            total |= WFC__A3D_GET(wave, nC0, nC1, pN) &
                WFC__A4D_GET(overlaps, -dC0 + n - 1, -dC1 + n - 1, p, pN);
        }

        WFC__A3D_GET(wave, c0, c1, p) = total;
    }

    int newAvailPattCnt = 0;
    for (int p = 0; p < pattCnt; ++p) {
        if (WFC__A3D_GET(wave, c0, c1, p)) ++newAvailPattCnt;
    }

    return oldAvailPattCnt != newAvailPattCnt;
}

bool wfc__propagateOntoNeighbours(
    int n, int options,
    int nC0, int nC1,
    const struct wfc__A4d_u8 overlaps,
    struct wfc__A2d_u8 ripple,
    struct wfc__A3d_u8 wave) {
    bool modified = false;

    for (int dC0 = -(n - 1); dC0 <= n - 1; ++dC0) {
        for (int dC1 = -(n - 1); dC1 <= n - 1; ++dC1) {
            if (((options & wfc__optEdgeFixC0) &&
                    (nC0 + dC0 < 0 || nC0 + dC0 >= wave.d03)) ||
                ((options & wfc__optEdgeFixC1) &&
                    (nC1 + dC1 < 0 || nC1 + dC1 >= wave.d13))) {
                continue;
            }

            if (wfc__propagateOntoDelta(n, nC0, nC1, dC0, dC1,
                    overlaps, wave)) {
                WFC__A2D_GET_WRAP(ripple, nC0 + dC0, nC1 + dC1) = 1;
                modified = true;
            }
        }
    }

    return modified;
}

void wfc__propagateFromRipple(
    int n, int options,
    const struct wfc__A4d_u8 overlaps,
    struct wfc__A2d_u8 ripple,
    struct wfc__A3d_u8 wave) {
    bool modified = true;
    while (modified) {
        modified = false;

        for (int nC0 = 0; nC0 < wave.d03; ++nC0) {
            for (int nC1 = 0; nC1 < wave.d13; ++nC1) {
                if (!WFC__A2D_GET(ripple, nC0, nC1)) continue;

                if (wfc__propagateOntoNeighbours(n, options, nC0, nC1,
                        overlaps, ripple, wave)) {
                    modified = true;
                }

                WFC__A2D_GET(ripple, nC0, nC1) = 0;
            }
        }
    }
}

void wfc__propagateFromAll(
    int n, int options,
    const struct wfc__A4d_u8 overlaps,
    struct wfc__A2d_u8 ripple,
    struct wfc__A3d_u8 wave) {
    memset(ripple.a, 1, WFC__A2D_SIZE(ripple));

    wfc__propagateFromRipple(n, options, overlaps, ripple, wave);
}

void wfc__propagateFromSeed(
    int n, int options,
    int seedC0, int seedC1,
    const struct wfc__A4d_u8 overlaps,
    struct wfc__A2d_u8 ripple,
    struct wfc__A3d_u8 wave) {
    memset(ripple.a, 0, WFC__A2D_SIZE(ripple));
    WFC__A2D_GET(ripple, seedC0, seedC1) = 1;

    wfc__propagateFromRipple(n, options, overlaps, ripple, wave);
}

int wfc__calcStatus(const struct wfc__A3d_u8 wave) {
    int minPatts = wave.d23, maxPatts = 0;
    for (int c0 = 0; c0 < wave.d03; ++c0) {
        for (int c1 = 0; c1 < wave.d13; ++c1) {
            int cntPatts = 0;
            for (int p = 0; p < wave.d23; ++p) {
                if (WFC__A3D_GET(wave, c0, c1, p)) ++cntPatts;
            }

            if (cntPatts < minPatts) minPatts = cntPatts;
            if (cntPatts > maxPatts) maxPatts = cntPatts;
        }
    }

    if (minPatts == 0) {
        // contradiction reached
        return wfc_failed;
    }
    if (maxPatts == 1) {
        return wfc_completed;
    }
    // still in progress
    return 0;
}

struct wfc_State {
    int status;
    void *ctx;
    int n, options, bytesPerPixel;
    int srcD0, srcD1, dstD0, dstD1;
    struct wfc__Pattern *patts;
    struct wfc__A4d_u8 overlaps;
    struct wfc__A3d_u8 wave;
    struct wfc__A2d_f entropies;
    struct wfc__A2d_u8 ripple;
};

int wfc_generate(
    int n, int options, int bytesPerPixel,
    int srcW, int srcH, const unsigned char *src,
    int dstW, int dstH, unsigned char *dst) {
    return wfc_generateEx(
        n, options, bytesPerPixel,
        srcW, srcH, src,
        dstW, dstH, dst,
        NULL, NULL
    );
}

int wfc_generateEx(
    int n, int options, int bytesPerPixel,
    int srcW, int srcH, const unsigned char *src,
    int dstW, int dstH, unsigned char *dst,
    void *ctx,
    bool *keep) {
    int ret = 0;

    wfc_State *state = wfc_initEx(n, options, bytesPerPixel,
        srcW, srcH, src, dstW, dstH, dst, ctx, keep);
    if (state == NULL) return wfc_callerError;

    while (!wfc_step(state));

    if (wfc_status(state) < 0) {
        ret = wfc_status(state);
    } else if (wfc_status(state) == wfc_completed) {
        int code = wfc_blit(state, src, dst);
        if (code != 0) ret = code;
    }

    wfc_free(state);

    return ret;
}

wfc_State* wfc_init(
    int n, int options, int bytesPerPixel,
    int srcW, int srcH, const unsigned char *src,
    int dstW, int dstH) {
    return wfc_initEx(
        n, options, bytesPerPixel,
        srcW, srcH, src,
        dstW, dstH, NULL,
        NULL, NULL
    );
}

wfc_State* wfc_initEx(
    int n, int options, int bytesPerPixel,
    int srcW, int srcH, const unsigned char *src,
    int dstW, int dstH, const unsigned char *dst,
    void *ctx,
    bool *keep) {
    if (n <= 0 ||
        bytesPerPixel <= 0 ||
        srcW <= 0 || srcH <= 0 || src == NULL ||
        dstW <= 0 || dstH <= 0 ||
        n > srcW || n > srcH || n > dstW || n > dstH) {
        return NULL;
    }
    if (keep != NULL && dst == NULL) {
        return NULL;
    }

    struct wfc__A3d_cu8 srcA = {srcH, srcW, bytesPerPixel, src};

    wfc_State *state = (wfc_State*)WFC_MALLOC(ctx, sizeof(*state));

    state->status = 0;
    state->ctx = ctx;
    state->n = n;
    state->options = options;
    state->bytesPerPixel = bytesPerPixel;
    state->srcD0 = srcH;
    state->srcD1 = srcW;
    state->dstD0 = dstH;
    state->dstD1 = dstW;

    int pattCnt;
    state->patts = wfc__gatherPatterns(ctx, n, options, srcA, &pattCnt);

    state->overlaps = wfc__calcOverlaps(ctx, n, srcA, pattCnt, state->patts);

    state->wave.d03 = dstH;
    if (options & wfc__optEdgeFixC0) state->wave.d03 -= n - 1;
    state->wave.d13 = dstW;
    if (options & wfc__optEdgeFixC1) state->wave.d13 -= n - 1;
    state->wave.d23 = pattCnt;
    state->wave.a = (uint8_t*)WFC_MALLOC(ctx, WFC__A3D_SIZE(state->wave));
    for (int i = 0; i < WFC__A3D_LEN(state->wave); ++i) state->wave.a[i] = 1;

    state->entropies.d02 = state->wave.d03;
    state->entropies.d12 = state->wave.d13;
    state->entropies.a = (float*)WFC_MALLOC(
        ctx, WFC__A2D_SIZE(state->entropies));

    state->ripple.d02 = state->wave.d03;
    state->ripple.d12 = state->wave.d13;
    state->ripple.a = (uint8_t*)WFC_MALLOC(ctx, WFC__A2D_SIZE(state->ripple));

    bool propagate = false;

    if (keep != NULL) {
        struct wfc__A3d_cu8 dstA =
            {state->dstD0, state->dstD1, state->bytesPerPixel, dst};
        struct wfc__A2d_b keepA = {state->dstD0, state->dstD1, keep};

        if (wfc__restrictKept(
                n, srcA, state->patts, dstA, keepA, state->wave)) {
            propagate = true;
        }
    }

    if (options & (wfc__optEdgeFixC0 | wfc__optEdgeFixC1)) {
        if (wfc__restrictEdges(options, state->patts, state->wave)) {
            propagate = true;
        }
    }

    if (propagate) {
        wfc__propagateFromAll(n, options,
            state->overlaps, state->ripple, state->wave);
        state->status = wfc__calcStatus(state->wave);
    }

    return state;
}

int wfc_status(const wfc_State *state) {
    if (state == NULL) return wfc_callerError;

    return state->status;
}

int wfc_step(wfc_State *state) {
    if (state == NULL) return wfc_callerError;

    if (state->status != 0) return state->status;

    int pattCnt = state->wave.d23;

    wfc__calcEntropies(state->patts, state->wave, state->entropies);

    int obsC0, obsC1;
    wfc__observeOne(
        state->ctx, pattCnt, state->patts, state->entropies, state->wave,
        &obsC0, &obsC1);

    wfc__propagateFromSeed(state->n, state->options,
        obsC0, obsC1,
        state->overlaps, state->ripple, state->wave);

    state->status = wfc__calcStatus(state->wave);

    return state->status;
}

int wfc_blit(
    const wfc_State *state,
    const unsigned char *src, unsigned char *dst) {
    if (state == NULL || state->status != wfc_completed ||
        src == NULL || dst == NULL) {
        return wfc_callerError;
    }

    struct wfc__A3d_cu8 srcA =
        {state->srcD0, state->srcD1, state->bytesPerPixel, src};
    struct wfc__A3d_u8 dstA =
        {state->dstD0, state->dstD1, state->bytesPerPixel, dst};

    int pattCnt = state->wave.d23;

    for (int c0 = 0; c0 < state->dstD0; ++c0) {
        for (int c1 = 0; c1 < state->dstD1; ++c1) {
            int wC0, wC1, pC0, pC1;
            wfc__coordsDstToWave(c0, c1, state->wave, &wC0, &wC1, &pC0, &pC1);

            int patt = 0;
            for (int p = 0; p < pattCnt; ++p) {
                if (WFC__A3D_GET(state->wave, wC0, wC1, p)) {
                    patt = p;
                    break;
                }
            }

            int sC0, sC1;
            wfc__coordsPattToSrc(
                state->n, state->patts[patt], pC0, pC1, srcA.d03, srcA.d13,
                &sC0, &sC1);

            const uint8_t *srcPx = &WFC__A3D_GET(srcA, sC0, sC1, 0);
            uint8_t *dstPx = &WFC__A3D_GET(dstA, c0, c1, 0);

            memcpy(dstPx, srcPx, (size_t)state->bytesPerPixel);
        }
    }

    return 0;
}

wfc_State* wfc_clone(const wfc_State *state) {
    if (state == NULL) return NULL;

    wfc_State *clone = (wfc_State*)WFC_MALLOC(state->ctx, sizeof(*clone));

    *clone = *state;

    int pattCnt = state->wave.d23;
    size_t pattsSz = (size_t)pattCnt * sizeof(*state->patts);
    clone->patts = (struct wfc__Pattern*)WFC_MALLOC(state->ctx, pattsSz);
    memcpy(clone->patts, state->patts, pattsSz);

    clone->overlaps.a = (uint8_t*)WFC_MALLOC(
        state->ctx, WFC__A4D_SIZE(state->overlaps));
    memcpy(clone->overlaps.a, state->overlaps.a,
        WFC__A4D_SIZE(state->overlaps));

    clone->wave.a = (uint8_t*)WFC_MALLOC(
        state->ctx, WFC__A3D_SIZE(state->wave));
    memcpy(clone->wave.a, state->wave.a,
        WFC__A3D_SIZE(state->wave));

    clone->entropies.a = (float*)WFC_MALLOC(
        state->ctx, WFC__A2D_SIZE(state->entropies));
    memcpy(clone->entropies.a, state->entropies.a,
        WFC__A2D_SIZE(state->entropies));

    clone->ripple.a = (uint8_t*)WFC_MALLOC(
        state->ctx, WFC__A2D_SIZE(state->ripple));
    memcpy(clone->ripple.a, state->ripple.a,
        WFC__A2D_SIZE(state->ripple));

    return clone;
}

void wfc_free(wfc_State *state) {
    if (state == NULL) return;

    void *ctx = state->ctx;
    (void)ctx;

    WFC_FREE(ctx, state->ripple.a);
    WFC_FREE(ctx, state->entropies.a);
    WFC_FREE(ctx, state->wave.a);
    WFC_FREE(ctx, state->overlaps.a);
    WFC_FREE(ctx, state->patts);
    WFC_FREE(ctx, state);
}

int wfc_patternCount(const wfc_State *state) {
    if (state == NULL) return wfc_callerError;

    return state->wave.d23;
}

int wfc_patternAvailable(const wfc_State *state, int patt, int x, int y) {
    if (state == NULL ||
        patt < 0 || patt >= state->wave.d23 ||
        x < 0 || x >= state->dstD1 ||
        y < 0 || y >= state->dstD0) {
        return wfc_callerError;
    }

    int wC0, wC1;
    wfc__coordsDstToWave(y, x, state->wave, &wC0, &wC1, NULL, NULL);

    return WFC__A3D_GET(state->wave, wC0, wC1, patt);
}

const unsigned char* wfc_pixelToBlit(
    const wfc_State *state,
    int patt, int x, int y, const unsigned char *src) {
    if (state == NULL ||
        patt < 0 || patt >= state->wave.d23 ||
        x < 0 || x >= state->dstD1 ||
        y < 0 || y >= state->dstD0 ||
        src == NULL) {
        return NULL;
    }

    struct wfc__A3d_cu8 srcA =
        {state->srcD0, state->srcD1, state->bytesPerPixel, src};

    int pC0, pC1;
    wfc__coordsDstToWave(y, x, state->wave, NULL, NULL, &pC0, &pC1);

    int sC0, sC1;
    wfc__coordsPattToSrc(
        state->n, state->patts[patt], pC0, pC1, srcA.d03, srcA.d13,
        &sC0, &sC1);

    return &WFC__A3D_GET(srcA, sC0, sC1, 0);
}

#endif // WFC_IMPLEMENTATION
