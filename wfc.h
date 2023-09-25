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

/*
This is a single-header library for the Wave Function Collapse algorithm (WFC).
WFC accepts a sample image, analyzes it, and generates a larger image that
resembles the input. It is a procedural image generation algorithm.

The easiest way to make use of this code is to use the provided CLI and GUI. If
you want to use the library itself, include it like this:

    #define WFC_IMPLEMENTATION
    #include "wfc.h"

You can then include it in other translation units by omitting the define
directive.

Run WFC with:

    wfc_generate(
        // pattern width and height, 3 is a good starting value
        n,
        // options to control WFC, 0 if you don't want to enable any
        wfc_optFlipH | wfc_optFlipV | wfc_optRotate,
        // byte size of a single pixel value
        4,
        // dimensions and bytes of the input image
        srcW, srcH, (unsigned char*)src,
        // dimensions and bytes of the output image
        dstW, dstH, (unsigned char*)dst);

This library does NOT handle file input/output and does NOT do backtracking on
its own. CLI and GUI do provide that, you may look at their code to see one
possible implementation.

You can also run WFC step-by-step like this:

    struct wfc_State *state = wfc_init(
        n, wfc_optFlipH | wfc_optFlipV | wfc_optRotate, 4,
        srcW, srcH, (unsigned char*)src,
        dstW, dstH);
    assert(state != NULL);

    while (!wfc_step(state));
    assert(wfc_status(state) > 0);

    int code = wfc_blit(state, (unsigned char*)src, (unsigned char*)dst);
    assert(code == 0);

    wfc_free(state);

wfc_clone() can be used to deep-copy a state object. You can use it to implement
your own backtracking behaviour.

WFC works by first gathering unique NxN patterns from the input image. You can
get the total number of patterns gathered with wfc_patternCount(). Use
wfc_patternPresentAt() to check if a pattern is still present at a particular
point. Use wfc_modifiedAt() to check if, during the previous step, a particular
point was modified, ie. its set of present patterns was reduced. Use
wfc_collapsedCount() to get the number of wave points collapsed to a single
pattern. Use wfc_pixelToBlitAt() to get a pointer to pixel bytes corresponding
to a particular pattern.

If you do not want to use standard C functions, you can define these macros
before including this header:

    #define WFC_ASSERT(ctx, cond) ...
    #define WFC_MALLOC(ctx, sz) ...
    #define WFC_FREE(ctx, p) ...
    // should yield a float value between 0 (inclusive) and 1 (exclusive)
    #define WFC_RAND(ctx) ...

All macros accept a user context pointer as the first argument. If you want it
to have a value other than null, you will need to supply that value by using
wfc_generateEx() or wfc_initEx().
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
    // This is a combination of wfc_optFlipH and wfc_optFlipV.
    wfc_optFlip = wfc_optFlipH | wfc_optFlipV,

    // Enable this option to allow rotating of patterns (by 90, 180, and 270
    // degrees).
    // @TODO Introduce separate rotation options for different angles.
    wfc_optRotate = 1 << 2,

    // Enable this option to fix left and right edges of input image so that
    // patterns may not wrap around them.
    wfc_optEdgeFixH = 1 << 4,
    // Enable this option to fix top and bottom edges of input image so that
    // patterns may not wrap around them.
    wfc_optEdgeFixV = 1 << 3,
    // This is a combination of wfc_optEdgeFixH and wfc_optEdgeFixV.
    wfc_optEdgeFix = wfc_optEdgeFixH | wfc_optEdgeFixV
};

// An opaque struct containing the WFC state. You should only interact with it
// through a pointer.
typedef struct wfc_State wfc_State;

/**
 * Runs WFC on the provided source image and blits to the destination.
 *
 * \param n Pattern size will be n by n pixels. Must be positive and not greater
 * than any dimension of source and destination images.
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
 * On success, the generated image will be written to dst.
 */
int wfc_generate(
    int n, int options, int bytesPerPixel,
    int srcW, int srcH, const unsigned char *src,
    int dstW, int dstH, unsigned char *dst);

/**
 * Runs WFC on the provided source image and blits to the destination. Same as
 * wfc_generate() except that it provides more capabilities.
 *
 * \param n Pattern size will be n by n pixels. Must be positive and not greater
 * than any dimension of source and destination images.
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
 * image. WFC output will be blitted here. If keep is non-null then this must
 * not be null.
 *
 * \param ctx User context that will be passed to WFC_ASSERT(), WFC_MALLOC(),
 * WFC_FREE(), and WFC_RAND().
 *
 * \param keep If non-null, signifies that some values in dst are pre-determined
 * and that WFC should not modify them. WFC will attempt to generate the rest of
 * the output image, while keeping these values as they are. If non-null, must
 * be of the same dimensions as dst - true means keep that pixel value
 * unchanged.
 *
 * \return Returns the status code of WFC, which is one of:
 *
 * \li wfc_completed (positive) in case of success;
 * \li wfc_failed (negative) in case of contradiction;
 * \li wfc_callerError (negative) in case of argument error.
 *
 * On success, the generated image will be written to dst.
 */
int wfc_generateEx(
    int n, int options, int bytesPerPixel,
    int srcW, int srcH, const unsigned char *src,
    int dstW, int dstH, unsigned char *dst,
    void *ctx,
    bool *keep);

/**
 * Allocates and initializes a state object for WFC. This is a first step
 * towards running WFC, you will likely be using wfc_step() after.
 *
 * Consider using wfc_generate() instead if you don't need any custom handling
 * of individual steps.
 *
 * \param n Pattern size will be n by n pixels. Must be positive and not greater
 * than any dimension of source and destination images.
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
 * functions. This object should be deallocated using wfc_free().
 *
 * In case of error, returns null.
 */
wfc_State* wfc_init(
    int n, int options, int bytesPerPixel,
    int srcW, int srcH, const unsigned char *src,
    int dstW, int dstH);

/**
 * Allocates and initializes a state object for WFC. This is a first step
 * towards running WFC, you will likely be using wfc_step() after. Same as
 * wfc_init() except that it provides more capabilities.
 *
 * Consider using wfc_generate() instead if you don't need any custom handling
 * of individual steps.
 *
 * \param n Pattern size will be n by n pixels. Must be positive and not greater
 * than any dimension of source and destination images.
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
 * images to be kept (see the keep parameter). If keep is non-null then this
 * must not be null.
 *
 * \param ctx User context that will be passed to WFC_ASSERT(), WFC_MALLOC(),
 * WFC_FREE(), and WFC_RAND().
 *
 * \param keep If non-null, signifies that some values in dst are pre-determined
 * and that WFC should not modify them. WFC will attempt to generate the rest of
 * the output image, while keeping these values as they are. If non-null, must
 * be of the same dimensions as dst - true means keep that pixel value
 * unchanged.
 *
 * \return Returns an allocated state object to be passed to further WFC
 * functions. This object should be deallocated using wfc_free().
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
 * wfc_step());
 * \li wfc_completed (positive) in case that WFC has completed successfully;
 * \li wfc_failed (negative) in case that WFC has reached a contradiction and
 * failed to complete;
 * \li wfc_callerError (negative) in case state was null.
*/
int wfc_status(const wfc_State *state);

/**
 * Performs one iteration of the WFC algorithm, observing one wave point and
 * propagating constraints. After WFC completes, you will likely be calling
 * wfc_blit() next.
 *
 * \param state State object pointer on which to perform the iteration. Must not
 * be null.
 *
 * \return Returns the status code after calling wfc_step(), which is one of:
 *
 * \li 0 (zero) in case that WFC has not completed yet (you can keep calling
 * wfc_step());
 * \li wfc_completed (positive) in case that WFC has completed successfully;
 * \li wfc_failed (negative) in case that WFC has reached a contradiction and
 * failed to complete;
 * \li wfc_callerError (negative) in case state was null.
*/
int wfc_step(wfc_State *state);

/**
 * Blits (aka. renders) the generated image to dst by copying in the pixel
 * values. Should be called after WFC completes successfully (after wfc_step()
 * has returned wfc_completed).
 *
 * \param state State object pointer. Must not be null. Must be in the completed
 * state.
 *
 * \param src Pointer to pixels comprising the source image. Must not be null.
 * Must be of the same dimensions as the image that was passed to wfc_init().
 *
 * \param dst Pointer to pixels comprising the destination image. Must not be
 * null. Must be of the dimensions that were specified in wfc_init().
 *
 * \return Returns zero on success or wfc_callerError if there was an error in
 * the arguments.
*/
int wfc_blit(
    const wfc_State *state,
    const unsigned char *src, unsigned char *dst);

/**
 * Allocates a new state object as a deep-copy of the provided one. The new
 * object is completely independent of the old one - you can call wfc_step() on
 * it and both objects need to be deallocated with wfc_free().
 *
 * \param state Pointer to the state object to be cloned.
 *
 * \return Returns a new state object that is a copy of the provided one. If
 * state is null, null is returned instead.
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
 * Returns the number of wave points collapsed to a single pattern.
 *
 * If a wave point has been reduced to a single pattern at some step, but at a
 * later step reduced to zero patterns (meaning WFC has failed), it will still
 * register towards this count.
 *
 * \param state State object pointer for which to query the number of collapsed
 * wave points. Must not be null.
 *
 * \return Returns the number of collapsed wave points. Returns wfc_callerError
 * (a negative value) if state is null.
*/
int wfc_collapsedCount(const wfc_State *state);

/**
 * Returns the number of unique patterns gathered from the input image when the
 * provided state object was being initialized.
 *
 * \param state State object pointer for which to query the number of unique
 * patterns. Must not be null.
 *
 * \return Returns the number of unique patterns gathered from the input image.
 * Returns wfc_callerError (a negative value) if state is null.
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
 * You can get the pattern count by calling wfc_patternCount().
 *
 * \param state State object pointer containing the wave point to be queried.
 * Must not be null.
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
int wfc_patternPresentAt(const wfc_State *state, int patt, int x, int y);

/**
 * Returns whether the wave point with the given coordinates has been modified
 * during the previous round of observation and propagations in wfc_step(). As
 * WFC iterates, patterns from wave points get removed as points are observed
 * and constraints are propagated - this function lets you know which points had
 * their set of remaining patterns reduced.
 *
 * If called before any calls to wfc_step() had been made, responds as if all
 * wave points have been modified.
 *
 * \param state State object pointer containing the wave point to be queried.
 * Must not be null.
 *
 * \param x x coordinate of the destination image. Must be a valid x coordinate
 * for the image being generated.
 *
 * \param y y coordinate of the destination image. Must be a valid y coordinate
 * for the image being generated.
 *
 * \return Returns:
 *
 * \li 0 if the wave point was not recently modified;
 * \li 1 if the wave point was modified during the previous round of observation
 * and propagations in wfc_step() or if no calls to wfc_step() have yet been
 * made;
 * \li wfc_callerError (a negative value) if there was an error in the
 * arguments.
*/
int wfc_modifiedAt(const wfc_State *state, int x, int y);

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
 * You can get the pattern count by calling wfc_patternCount().
 *
 * \param state State object pointer containing the wave point to be queried.
 * Must not be null.
 *
 * \param src Pointer to pixels comprising the source image. Must not be null.
 * Must be of the same dimensions as the image that was passed to wfc_init().
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
 * \return Returns a pointer to the bytes within src for the corresponding
 * pixel value. Returns null if there was an error in the arguments.
*/
const unsigned char* wfc_pixelToBlitAt(
    const wfc_State *state, const unsigned char *src,
    int patt, int x, int y);

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

// Counts the number of 1 bits in a value.
int wfc__popcount_u(unsigned n) {
    int cnt = 0;
    for (; n != 0; ++cnt) n &= n - 1;

    return cnt;
}

int wfc__roundUpToDivBy(int n, int div) {
    return ((n + div - 1) / div) * div;
}

// a and b must be non-negative.
// Assumes IEEE 754 representation of float on the system.
bool wfc__approxEqNonNeg_f(float a, float b) {
    const int ulpsDiff = 8;

    // Reinterpret floats as 32-bit ints in a standard compliant way.
    int32_t ia, ib;
    memcpy(&ia, &a, sizeof(a));
    memcpy(&ib, &b, sizeof(b));

    // Check that bit representations are close to each other.
    return abs(ia - ib) < ulpsDiff;
}

// Approximates log2(x), where x is positive
// and not NaN, infinity, nor a subnormal.
// Assumes IEEE 754 representation of float on the system.
float wfc__log2f(float x) {
    // IEEE 754 representation constants.
    const int32_t mantissaLen = 23;
    const int32_t mantissaMask = (1 << mantissaLen) - 1;
    const int32_t baseExponent = -127;

    // Reinterpret x as int in a standard compliant way.
    int32_t xu;
    memcpy(&xu, &x, sizeof(xu));

    // Calculate exponent of x.
    float e = (float)((xu >> mantissaLen) + baseExponent);

    // Calculate mantissa of x. It will be in range [1, 2).
    float m;
    int32_t mxu = (xu & mantissaMask) | ((-baseExponent) << mantissaLen);
    memcpy(&m, &mxu, sizeof(m));

    // Use Remez algorithm-generated 3rd degree approximation polynomial
    // for log2(a) where a is in range [1, 2].
    float l = 0.15824871f;
    l = l * m + -1.051875f;
    l = l * m + 3.0478842f;
    l = l * m + -2.1536207f;

    // Add exponent to the calculation.
    // Final log is log2(m*2^e)=log2(m)+e.
    l += e;

    return l;
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

// Wraps ind into range [0, sz).
int wfc__indWrap(int ind, int sz) {
    if (ind >= 0) return ind % sz;
    return sz + ind % sz;
}

// multi-dimensional array utility

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

    if (c0 != NULL) *c0 = c0_;
    if (c1 != NULL) *c1 = c1_;
}

int wfc__coords2dToInd(int d1, int c0, int c1) {
    return c0 * d1 + c1;
}

int wfc__bitPackLen(int cnt) {
    const int uSzBits = (int)sizeof(unsigned) * 8;

    return wfc__roundUpToDivBy(cnt, uSzBits) / uSzBits;
}

bool wfc__getBit(const unsigned *a, int ind) {
    const int uSzBits = (int)sizeof(unsigned) * 8;

    const unsigned bitMask = 1u << (unsigned)(ind % uSzBits);

    return a[ind / uSzBits] & bitMask;
}

void wfc__setBit(unsigned *a, int ind, bool val) {
    const int uSzBits = (int)sizeof(unsigned) * 8;

    const unsigned bitMask = 1u << (unsigned)(ind % uSzBits);

    if (val) a[ind / uSzBits] |= bitMask;
    else a[ind / uSzBits] &= ~bitMask;
}

// uint8_ts are used instead of bools for performance concerns.
WFC__A2D_DEF(bool, b);
WFC__A2D_DEF(uint8_t, u8);
WFC__A2D_DEF(int, i);
WFC__A2D_DEF(float, f);
WFC__A3D_DEF(uint8_t, u8);
WFC__A3D_DEF(const uint8_t, cu8);
WFC__A3D_DEF(unsigned, u);

// WFC code

// H and V are used in public API, prefer to use C0/1/... in private code.
enum {
    wfc__optFlipC0 = wfc_optFlipV,
    wfc__optFlipC1 = wfc_optFlipH,
    wfc__optEdgeFixC0 = wfc_optEdgeFixV,
    wfc__optEdgeFixC1 = wfc_optEdgeFixH
};

// Transformations for patterns are encoded in a bitmask.
// @TODO Some combinations are equivalent, so work in gathering patterns gets
// duplicated. Optimize by removing that work duplication.
enum {
    wfc__tfFlipC0 = 1 << 0,
    wfc__tfFlipC1 = 1 << 1,
    wfc__tfRot90 = 1 << 2,
    wfc__tfRot180 = 1 << 3,
    // Rotating by 270 is a combination of rotation by 90 and 180.
    wfc__tfRot270 = wfc__tfRot90 | wfc__tfRot180,

    wfc__tfCnt = 1 << 4
};

enum wfc__Dir {
    wfc__dirC0Less,
    wfc__dirC0More,
    wfc__dirC1Less,
    wfc__dirC1More,

    wfc__dirCnt
};

void wfc__dirToOffsets(void *ctx, enum wfc__Dir dir, int *offC0, int *offC1) {
    (void)ctx;

    int offC0_ = 0, offC1_ = 0;
    if (dir == wfc__dirC0Less) {
        offC0_ = -1;
    } else if (dir == wfc__dirC0More) {
        offC0_ = +1;
    } else if (dir == wfc__dirC1Less) {
        offC1_ = -1;
    } else if (dir == wfc__dirC1More) {
        offC1_ = +1;
    } else {
        WFC_ASSERT(ctx, false);
    }

    if (offC0 != NULL) *offC0 = offC0_;
    if (offC1 != NULL) *offC1 = offC1_;
}

enum wfc__Dir wfc__dirOpposite(void *ctx, enum wfc__Dir dir) {
    (void)ctx;

    if (dir == wfc__dirC0Less) {
        return wfc__dirC0More;
    } else if (dir == wfc__dirC0More) {
        return wfc__dirC0Less;
    } else if (dir == wfc__dirC1Less) {
        return wfc__dirC1More;
    } else if (dir == wfc__dirC1More) {
        return wfc__dirC1Less;
    } else {
        WFC_ASSERT(ctx, false);
    }

    // Unreachable.
    return (enum wfc__Dir)0;
}

void wfc__coords2dPlusDir(
    void *ctx, int c0, int c1, enum wfc__Dir dir, int *rC0, int *rC1) {
    int offC0, offC1;
    wfc__dirToOffsets(ctx, dir, &offC0, &offC1);

    int rC0_ = c0 + offC0;
    int rC1_ = c1 + offC1;

    if (rC0 != NULL) *rC0 = rC0_;
    if (rC1 != NULL) *rC1 = rC1_;
}

bool wfc__getBitA3d(
    const struct wfc__A3d_u arr, int c0, int c1, int c2) {
    return wfc__getBit(&WFC__A3D_GET(arr, c0, c1, 0), c2);
}

void wfc__setBitA3d(
    const struct wfc__A3d_u arr, int c0, int c1, int c2, bool val) {
    wfc__setBit(&WFC__A3D_GET(arr, c0, c1, 0), c2, val);
}

// Bit packs may have surplus bit positions that need to equal 0.
// That is why a function to set all bits to 1 is not provided
// as it would be easy to make the mistake of setting surplus bits to 1.
void wfc__clearBitPackA3d(
    const struct wfc__A3d_u arr, int c0, int c1) {
    memset(&WFC__A3D_GET(arr, c0, c1, 0), 0, (size_t)arr.d23 * sizeof(*arr.a));
}

int wfc__popcountBitPackA3d(
    const struct wfc__A3d_u arr, int c0, int c1) {
    int cnt = 0;
    for (int i = 0; i < arr.d23; ++i) {
        cnt += wfc__popcount_u(WFC__A3D_GET(arr, c0, c1, i));
    }

    return cnt;
}

struct wfc__Pattern {
    // Coordinates of the top-left pixel in the source image.
    int c0, c1;
    // Bitmask for transformation done to produce this pattern.
    int tf;

    // Whether this pattern may be placed along a particular edge in output.
    bool edgeC0Lo, edgeC0Hi, edgeC1Lo, edgeC1Hi;
    // How often this pattern appeared in different places in the source image.
    int freq;
};

void wfc__coordsPattToSrc(
    int n,
    struct wfc__Pattern patt, int pC0, int pC1,
    int sD0, int sD1, int *sC0, int *sC1) {
    int tfC0 = pC0;
    int tfC1 = pC1;

    // Map from pattern space to source space.
    {
        if (patt.tf & wfc__tfFlipC0) tfC0 = n - 1 - tfC0;
        if (patt.tf & wfc__tfFlipC1) tfC1 = n - 1 - tfC1;

        // Rot-270 is rot-90 plus rot-180 both in bitmask and as transformation.
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

// Fill in edge info in a pattern.
void wfc__fillPattEdges(
    int n, int sD0, int sD1,
    struct wfc__Pattern *patt) {
    bool edgeC0Lo = patt->c0 == 0;
    bool edgeC0Hi = patt->c0 + n == sD0;
    bool edgeC1Lo = patt->c1 == 0;
    bool edgeC1Hi = patt->c1 + n == sD1;

    // Map from source space to pattern space.
    // Note that this is the inverse of what is done in wfc__coordsPattToSrc().
    {
        // Rot-270 is rot-90 plus rot-180 both in bitmask and as transformation.
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

// Maps from destination coordinates to wave coordinates.
// When edges are fixed, wave may be shorter and/or narrower than destination,
// so multiple destination points will map to the same wave point,
// but with different offsets.
void wfc__coordsDstToWave(
    int dC0, int dC1,
    const struct wfc__A3d_u wave,
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

// Returns the number of the highest possible number of unique patterns.
int wfc__pattCombCnt(int d0, int d1) {
    return d0 * d1 * wfc__tfCnt;
}

// Fills in a pattern based on the pattern index.
// Pattern index encodes its position in the destination
// along with the transformations to be used to create it.
// This function maps from index to actual pattern data.
// Only sets pattern coordinates and transformations.
void wfc__indToPattComb(int d1, int ind, struct wfc__Pattern *patt) {
    wfc__indToCoords2d(d1, ind / wfc__tfCnt, &patt->c0, &patt->c1);
    patt->tf = ind % wfc__tfCnt;
}

// Returns whether the pattern used only allowed transformations
// and was picked from an allowed position in the source.
bool wfc__satisfiesOptions(
    int n, int options, int sD0, int sD1,
    struct wfc__Pattern patt) {
    if ((patt.tf & wfc__tfFlipC0) && !(options & wfc__optFlipC0)) return false;
    if ((patt.tf & wfc__tfFlipC1) && !(options & wfc__optFlipC1)) return false;

    if ((patt.tf & (wfc__tfRot90 | wfc__tfRot180 | wfc__tfRot270)) &&
        !(options & wfc_optRotate)) {
        return false;
    }

    // When an edge is fixed, patterns are not allowed to wrap around it.
    if ((options & wfc__optEdgeFixC0) && patt.c0 + n > sD0) return false;
    if ((options & wfc__optEdgeFixC1) && patt.c1 + n > sD1) return false;

    return true;
}

// Returns whether two patterns contain the same underlying subimage.
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

    // First, determine the number of unique patterns.
    // This is done by iterating through all patterns
    // and then through all patterns before that one, comparing the two.
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

    // Now that we know the number of unique patterns,
    // we can allocate an array for them and start collecting them.
    // Collection still works by iterating through all patterns
    // and then through all patterns before that one, comparing the two.
    // If no previous pattern is equal, collect the new pattern.
    patts = (struct wfc__Pattern*)WFC_MALLOC(
        ctx, (size_t)pattCnt * sizeof(*patts));
    int pattInd = 0;
    for (int i = 0; i < wfc__pattCombCnt(src.d03, src.d13); ++i) {
        struct wfc__Pattern patt = {0, 0, 0, 0, 0, 0, 0, 0};
        wfc__indToPattComb(src.d13, i, &patt);
        if (!wfc__satisfiesOptions(n, options, src.d03, src.d13, patt)) {
            continue;
        }

        // We now need to fill in all pattern fields.
        wfc__fillPattEdges(n, src.d03, src.d13, &patt);
        patt.freq = 1;

        bool seenBefore = false;
        for (int i1 = 0; !seenBefore && i1 < pattInd; ++i1) {
            if (wfc__patternsEq(n, src, patt, patts[i1])) {
                // If the patterns are equal, we know this is NOT a new pattern.
                // However, it may have been placed along a different edge,
                // which means the old pattern may also be placed along it.
                // So, update the edge info of the old pattern.
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

// Returns whether the subimage overlap between patterns A and B match up,
// where pattern B is directly at the given direction away from pattern A.
bool wfc__overlapMatches(
    void *ctx,
    int n, const struct wfc__A3d_cu8 src,
    enum wfc__Dir dir,
    struct wfc__Pattern pattA, struct wfc__Pattern pattB) {
    (void)ctx;

    int offC0, offC1;
    wfc__dirToOffsets(ctx, dir, &offC0, &offC1);

    // Sizes of the overlap area.
    int overlapD0 = n - abs(offC0);
    int overlapD1 = n - abs(offC1);

    // Coordinates of the overlap area in pattern A and pattern B spaces.
    int overlapPAC0 = offC0 > 0 ? offC0 : 0;
    int overlapPAC1 = offC1 > 0 ? offC1 : 0;
    int overlapPBC0 = offC0 > 0 ? 0 : abs(offC0);
    int overlapPBC1 = offC1 > 0 ? 0 : abs(offC1);

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

struct wfc__A3d_u wfc__calcOverlaps(
    void *ctx,
    int n, const struct wfc__A3d_cu8 src,
    int pattCnt, const struct wfc__Pattern *patts) {
    struct wfc__A3d_u overlaps;
    overlaps.d03 = wfc__dirCnt;
    overlaps.d13 = pattCnt;
    overlaps.d23 = wfc__bitPackLen(pattCnt);
    overlaps.a = (unsigned*)WFC_MALLOC(ctx, WFC__A3D_SIZE(overlaps));

    memset(overlaps.a, 0, WFC__A3D_SIZE(overlaps));
    for (int dir = 0; dir < wfc__dirCnt; ++dir) {
        for (int i = 0; i < pattCnt; ++i) {
            for (int j = 0; j < pattCnt; ++j) {
                bool overlap = wfc__overlapMatches(ctx, n, src,
                    (enum wfc__Dir)dir, patts[i], patts[j]);
                wfc__setBitA3d(overlaps, dir, i, j, overlap);
            }
        }
    }

    return overlaps;
}

bool wfc__restrictKept(
    int n,
    const struct wfc__A3d_cu8 src,
    int pattCnt, const struct wfc__Pattern *patts,
    const struct wfc__A3d_cu8 dst,
    const struct wfc__A2d_b keep,
    struct wfc__A3d_u wave) {
    const int bytesPerPixel = dst.d23;

    bool modif = false;

    for (int wC0 = 0; wC0 < wave.d03; ++wC0) {
        for (int wC1 = 0; wC1 < wave.d13; ++wC1) {
            for (int i = 0; i < n; ++i) {
                for (int j = 0; j < n; ++j) {
                    int dC0 = wfc__indWrap(wC0 + i, dst.d03);
                    int dC1 = wfc__indWrap(wC1 + j, dst.d13);

                    if (!WFC__A2D_GET(keep, dC0, dC1)) continue;

                    const unsigned char *dPx = &WFC__A3D_GET(dst, dC0, dC1, 0);

                    for (int p = 0; p < pattCnt; ++p) {
                        if (!wfc__getBitA3d(wave, wC0, wC1, p)) continue;

                        int sC0, sC1;
                        wfc__coordsPattToSrc(
                            n,
                            patts[p], i, j,
                            src.d03, src.d13,
                            &sC0, &sC1);

                        const unsigned char *sPx =
                            &WFC__A3D_GET(src, sC0, sC1, 0);

                        if (memcmp(dPx, sPx, (size_t)bytesPerPixel) != 0) {
                            wfc__setBitA3d(wave, wC0, wC1, p, false);
                            modif = true;
                        }
                    }
                }
            }
        }
    }

    return modif;
}

bool wfc__restrictEdges(
    int options,
    int pattCnt, const struct wfc__Pattern *patts,
    struct wfc__A3d_u wave) {
    const int d0 = wave.d03, d1 = wave.d13;

    bool modif = false;

    if (options & wfc__optEdgeFixC0) {
        for (int i = 0; i < d1; ++i) {
            for (int p = 0; p < pattCnt; ++p) {
                if (wfc__getBitA3d(wave, 0, i, p) && !patts[p].edgeC0Lo) {
                    wfc__setBitA3d(wave, 0, i, p, false);
                    modif = true;
                }
                if (wfc__getBitA3d(wave, d0 - 1, i, p) && !patts[p].edgeC0Hi) {
                    wfc__setBitA3d(wave, d0 - 1, i, p, false);
                    modif = true;
                }
            }
        }
    }
    if (options & wfc__optEdgeFixC1) {
        for (int i = 0; i < d0; ++i) {
            for (int p = 0; p < pattCnt; ++p) {
                if (wfc__getBitA3d(wave, i, 0, p) && !patts[p].edgeC1Lo) {
                    wfc__setBitA3d(wave, i, 0, p, false);
                    modif = true;
                }
                if (wfc__getBitA3d(wave, i, d1 - 1, p) && !patts[p].edgeC1Hi) {
                    wfc__setBitA3d(wave, i, d1 - 1, p, false);
                    modif = true;
                }
            }
        }
    }

    return modif;
}

void wfc__calcEntropies(
    int pattCnt, const struct wfc__Pattern *patts,
    const struct wfc__A3d_u wave,
    const struct wfc__A2d_u8 modified,
    struct wfc__A2d_f entropies) {
    for (int c0 = 0; c0 < wave.d03; ++c0) {
        for (int c1 = 0; c1 < wave.d13; ++c1) {
            if (!WFC__A2D_GET(modified, c0, c1)) continue;

            int totalFreq = 0;
            int presentPatts = 0;
            for (int p = 0; p < pattCnt; ++p) {
                if (wfc__getBitA3d(wave, c0, c1, p)) {
                    totalFreq += patts[p].freq;
                    ++presentPatts;
                }
            }

            float entropy;
            if (presentPatts > 1) {
                entropy = 0;
                for (int p = 0; p < pattCnt; ++p) {
                    if (wfc__getBitA3d(wave, c0, c1, p)) {
                        float prob = (float)patts[p].freq / (float)totalFreq;
                        entropy -= prob * wfc__log2f(prob);
                    }
                }
            } else {
                // Entropy of collapsed points is set to the largest float.
                // This does not adhere to the Shannon entropy formula,
                // but speeds up finding points tied for the smallest entropy.
                entropy = FLT_MAX;
            }

            WFC__A2D_GET(entropies, c0, c1) = entropy;
        }
    }
}

void wfc__observeOne(
    void *ctx,
    int pattCnt, const struct wfc__Pattern *patts,
    const struct wfc__A2d_f entropies,
    struct wfc__A3d_u wave,
    struct wfc__A2d_u8 modified,
    int *obsC0, int *obsC1) {
    // Collapsed points have entropy set to the largest float.
    // It is in practice impossible for that value
    // to end up registering as (almost) equal to real entropy values.
    // If all points are collapsed, this function will not get called.
    float smallest = entropies.a[0];
    // The number of different wave points tied for the smallest entropy.
    int smallestCnt = 1;
    for (int i = 1; i < WFC__A2D_LEN(entropies); ++i) {
        if (wfc__approxEqNonNeg_f(entropies.a[i], smallest)) {
            ++smallestCnt;
        } else if (entropies.a[i] < smallest) {
            smallest = entropies.a[i];
            smallestCnt = 1;
        }
    }

    int chosenC0, chosenC1;
    {
        int chosenPnt = 0;
        // Pick which point tied for the lowest entropy to observe.
        int chosenSmallestPnt = wfc__rand_i(ctx, smallestCnt);
        // Iterate through points until we get to the one we decided to observe.
        for (int i = 0; i < WFC__A2D_LEN(entropies); ++i) {
            if (wfc__approxEqNonNeg_f(entropies.a[i], smallest)) {
                chosenPnt = i;
                if (chosenSmallestPnt == 0) break;
                --chosenSmallestPnt;
            }
        }
        wfc__indToCoords2d(entropies.d12, chosenPnt, &chosenC0, &chosenC1);
    }

    // Now pick a pattern to collapse the chosen point into.
    // Picks based on pattern frequencies as weights.
    int chosenPatt = 0;
    {
        int totalFreq = 0;
        for (int i = 0; i < pattCnt; ++i) {
            if (wfc__getBitA3d(wave, chosenC0, chosenC1, i)) {
                totalFreq += patts[i].freq;
            }
        }
        int chosenInst = wfc__rand_i(ctx, totalFreq);

        for (int i = 0; i < pattCnt; ++i) {
            if (wfc__getBitA3d(wave, chosenC0, chosenC1, i)) {
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
    wfc__clearBitPackA3d(wave, chosenC0, chosenC1);
    wfc__setBitA3d(wave, chosenC0, chosenC1, chosenPatt, true);
    WFC__A2D_GET(modified, chosenC0, chosenC1) = 1;
}

// Propagate constraints from a recently modified point
// onto the neighbouring one in a particular direction.
// Returns whether the neighbouring point was modified.
bool wfc__propagateOntoDirection(
    void *ctx, int options, int pattCnt,
    int c0, int c1, enum wfc__Dir dir,
    const struct wfc__A3d_u overlaps,
    struct wfc__A3d_u wave) {
    int nC0, nC1;
    wfc__coords2dPlusDir(ctx, c0, c1, dir, &nC0, &nC1);

    // Constraints are not propagated along fixed edges.
    if (((options & wfc__optEdgeFixC0) && (nC0 < 0 || nC0 >= wave.d03)) ||
        ((options & wfc__optEdgeFixC1) && (nC1 < 0 || nC1 >= wave.d13))) {
        return false;
    }

    nC0 = wfc__indWrap(nC0, wave.d03);
    nC1 = wfc__indWrap(nC1, wave.d13);

    int dirOpposite = (int)wfc__dirOpposite(ctx, dir);

    // We will compare the old and new pattern count at the neighbouring point
    // to know whether we modified it.
    int oldPresentPattCnt = wfc__popcountBitPackA3d(wave, nC0, nC1);

    // For each pattern at the neighbouring point
    // figure out whether it can be kept,
    // which is the case if there is a pattern at starting point
    // whose overlap matches.
    for (int p = 0; p < pattCnt; ++p) {
        if (!wfc__getBitA3d(wave, nC0, nC1, p)) continue;

        // This is a very nested and hot loop in the code,
        // so a few optimizations were made.
        // All changes should be verified with benchmarks.
        unsigned total = 0;
        for (int i = 0; i < wave.d23; ++i) {
            total |= WFC__A3D_GET(wave, c0, c1, i) &
                WFC__A3D_GET(overlaps, dirOpposite, p, i);
        }

        wfc__setBitA3d(wave, nC0, nC1, p, total);
    }

    int newPresentPattCnt = wfc__popcountBitPackA3d(wave, nC0, nC1);

    return oldPresentPattCnt != newPresentPattCnt;
}

void wfc__propagateFromRipple(
    void *ctx, int n, int options, int pattCnt,
    const struct wfc__A3d_u overlaps,
    int head, int tail, struct wfc__A2d_i ripple,
    struct wfc__A3d_u wave,
    struct wfc__A2d_u8 modified) {
    // If patterns are 1x1, they never overlap
    // and points never constrain each other.
    if (n == 1) return;

    // Constraints only need to be propagated from recently modified points.
    // As additional wave points are constrained,
    // constraint propagation repeats from those points
    // causing the propagation to keep going.

    // The way ripple is used is as a linked list represented with an array
    // whose elements contain indexes of the next linked element.
    // Negative index means no next elements (sort of like a null pointer).
    // Each element corresponds to a wave point with the same coordinates.
    // If a point is inside this linked list,
    // that means constraints need to be propagated from that point.

    // In each iteration, constraints are propagated from head.
    // New points are added after tail
    // if they become modified and are not already in the list.
    // Propagation ends when the list is empty.
    while (head >= 0) {
        // This function uses both raw 1D array indexes and full coordinates.
        int headC0, headC1;
        wfc__indToCoords2d(ripple.d12, head, &headC0, &headC1);

        // Only propagate to the cardinally adjacent neighbours.
        // All constraints will eventually be propagated,
        // but with extra iterations in between.
        // This is still a significant performance improvement.
        for (int dir = 0; dir < wfc__dirCnt; ++dir) {
            if (wfc__propagateOntoDirection(
                    ctx, options, pattCnt,
                    headC0, headC1, (enum wfc__Dir)dir,
                    overlaps, wave)) {
                int nextC0, nextC1;
                wfc__coords2dPlusDir(
                    ctx, headC0, headC1, (enum wfc__Dir)dir, &nextC0, &nextC1);
                nextC0 = wfc__indWrap(nextC0, ripple.d02);
                nextC1 = wfc__indWrap(nextC1, ripple.d12);

                int next = wfc__coords2dToInd(ripple.d12, nextC0, nextC1);

                // If next was modified and is not in the list,
                // add it to the list to be propagated from later on.
                // We know that an element is not in the list
                // if it's not pointing to anything and is not the tail.
                if (ripple.a[next] < 0 && next != tail) {
                    ripple.a[tail] = next;
                    tail = next;
                }

                WFC__A2D_GET(modified, nextC0, nextC1) = 1;
            }
        }

        // Remove head from the list
        // as we just propagated all we wanted from it.
        // Remove it by setting head to its next element
        // and having the old head no longer point to anything.
        int newHead = ripple.a[head];
        ripple.a[head] = -1;
        head = newHead;
    }
}

void wfc__propagateFromAll(
    void *ctx, int n, int options, int pattCnt,
    const struct wfc__A3d_u overlaps,
    struct wfc__A2d_i ripple,
    struct wfc__A3d_u wave,
    struct wfc__A2d_u8 modified) {
    // The linked list will contain all elements in order.
    // Each element will point to the next one,
    // except for the last element, which will be the tail.
    int head = 0, tail = WFC__A2D_LEN(ripple) - 1;
    for (int i = head; i < tail; ++i) {
        ripple.a[i] = i + 1;
    }
    ripple.a[tail] = -1;

    wfc__propagateFromRipple(
        ctx, n, options, pattCnt, overlaps, head, tail, ripple, wave, modified);
}

void wfc__propagateFromSeed(
    void *ctx, int n, int options, int pattCnt,
    int seedC0, int seedC1,
    const struct wfc__A3d_u overlaps,
    struct wfc__A2d_i ripple,
    struct wfc__A3d_u wave,
    struct wfc__A2d_u8 modified) {
    // Only one element will be in the linked list
    // and will be both the head and the tail.
    // No one has a next element to point to.
    for (int i = 0; i < WFC__A2D_LEN(ripple); ++i) {
        ripple.a[i] = -1;
    }
    int head = wfc__coords2dToInd(ripple.d12, seedC0, seedC1), tail = head;

    wfc__propagateFromRipple(
        ctx, n, options, pattCnt, overlaps, head, tail, ripple, wave, modified);
}

void wfc__updateCnts(
    const struct wfc__A3d_u wave,
    const struct wfc__A2d_u8 modified,
    struct wfc__A2d_i wavePattCnts,
    int *collapsedCnt) {
    for (int c0 = 0; c0 < wave.d03; ++c0) {
        for (int c1 = 0; c1 < wave.d13; ++c1) {
            if (!WFC__A2D_GET(modified, c0, c1)) continue;

            int cntPatts = wfc__popcountBitPackA3d(wave, c0, c1);

            WFC__A2D_GET(wavePattCnts, c0, c1) = cntPatts;
            if (cntPatts == 1) ++(*collapsedCnt);
        }
    }
}

int wfc__calcStatus(int pattCnt, const struct wfc__A2d_i wavePattCnts) {
    int minPatts = pattCnt, maxPatts = 0;
    for (int c0 = 0; c0 < wavePattCnts.d02; ++c0) {
        for (int c1 = 0; c1 < wavePattCnts.d12; ++c1) {
            int cnt = WFC__A2D_GET(wavePattCnts, c0, c1);

            minPatts = wfc__min_i(minPatts, cnt);
            maxPatts = wfc__max_i(maxPatts, cnt);
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
    // User context.
    void *ctx;
    int n, options, bytesPerPixel;
    int srcD0, srcD1, dstD0, dstD1;
    // Number of collapsed wave points.
    int collapsedCnt;
    // Number of collected patterns.
    int pattCnt;
    // Patterns collected from source.
    struct wfc__Pattern *patts;
    // Whether, in a particular direction (first index),
    // two patterns (second index and bit pack position)
    // have matching subimage pixel values.
    // wfc__Dir is used for the first index.
    // This is a series of bit packs stored as arrays of unsigned.
    // Ergo, booleans are represented as bits and tightly packed.
    // Use bit pack utility functions when working with this array.
    struct wfc__A3d_u overlaps;
    // Whether, for each point (first two coordinates),
    // a particular pattern (bit pack position)
    // is still present.
    // This is a series of bit packs stored as arrays of unsigned.
    // Ergo, booleans are represented as bits and tightly packed.
    // Use bit pack utility functions when working with this array.
    struct wfc__A3d_u wave;
    // Number of remaining patterns on corresponding wave points.
    struct wfc__A2d_i wavePattCnts;
    // Allocated once and reused when new entropy values are calculated.
    struct wfc__A2d_f entropies;
    // Array of bools that tells which wave points were modified
    // in the last round of observation and propagation.
    // Allocated once and reused in all propagation calls.
    struct wfc__A2d_u8 modified;
    // Scratch space used for constraint propagation.
    // Check out propagation code to understand how it's used.
    // Allocated once and reused in all propagation calls.
    struct wfc__A2d_i ripple;
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

// All allocations happen during initialization.
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
    state->collapsedCnt = 0;

    state->patts = wfc__gatherPatterns(ctx, n, options, srcA, &state->pattCnt);

    state->overlaps = wfc__calcOverlaps(
        ctx, n, srcA, state->pattCnt, state->patts);

    state->wave.d03 = dstH;
    if (options & wfc__optEdgeFixC0) state->wave.d03 -= n - 1;
    state->wave.d13 = dstW;
    if (options & wfc__optEdgeFixC1) state->wave.d13 -= n - 1;
    state->wave.d23 = wfc__bitPackLen(state->pattCnt);
    state->wave.a = (unsigned*)WFC_MALLOC(ctx, WFC__A3D_SIZE(state->wave));
    // Set all patterns as present.
    memset(state->wave.a, 0xFF, WFC__A3D_SIZE(state->wave));
    // Surplus bit pack positions don't correspond to any real patterns
    // so they get set to false.
    {
        const int pLo = state->pattCnt;
        const int pHi = state->wave.d23 * (int)sizeof(*state->wave.a) * 8;
        for (int c0 = 0; c0 < state->wave.d03; ++c0) {
            for (int c1 = 0; c1 < state->wave.d13; ++c1) {
                for (int p = pLo; p < pHi; ++p) {
                    wfc__setBitA3d(state->wave, c0, c1, p, false);
                }
            }
        }
    }

    state->wavePattCnts.d02 = state->wave.d03;
    state->wavePattCnts.d12 = state->wave.d13;
    state->wavePattCnts.a = (int*)WFC_MALLOC(
        ctx, WFC__A2D_SIZE(state->wavePattCnts));

    state->entropies.d02 = state->wave.d03;
    state->entropies.d12 = state->wave.d13;
    state->entropies.a = (float*)WFC_MALLOC(
        ctx, WFC__A2D_SIZE(state->entropies));

    state->modified.d02 = state->wave.d03;
    state->modified.d12 = state->wave.d13;
    state->modified.a = (uint8_t*)WFC_MALLOC(
        ctx, WFC__A2D_SIZE(state->modified));
    memset(state->modified.a, 1, WFC__A2D_SIZE(state->modified));

    state->ripple.d02 = state->wave.d03;
    state->ripple.d12 = state->wave.d13;
    state->ripple.a = (int*)WFC_MALLOC(ctx, WFC__A2D_SIZE(state->ripple));

    // Usually, all patterns are present in all wave points,
    // unless some extra options were used.
    // Don't needlessly try to propagate in the usual case.
    bool propagate = false;

    if (keep != NULL) {
        struct wfc__A3d_cu8 dstA =
            {state->dstD0, state->dstD1, state->bytesPerPixel, dst};
        struct wfc__A2d_b keepA = {state->dstD0, state->dstD1, keep};

        if (wfc__restrictKept(
                n, srcA,
                state->pattCnt, state->patts,
                dstA, keepA, state->wave)) {
            propagate = true;
        }
    }

    if (options & (wfc__optEdgeFixC0 | wfc__optEdgeFixC1)) {
        if (wfc__restrictEdges(
                options, state->pattCnt, state->patts, state->wave)) {
            propagate = true;
        }
    }

    if (propagate) {
        wfc__propagateFromAll(
            ctx, n, options, state->pattCnt,
            state->overlaps, state->ripple, state->wave, state->modified);
    }

    wfc__updateCnts(state->wave, state->modified,
        state->wavePattCnts, &state->collapsedCnt);
    state->status = wfc__calcStatus(state->pattCnt, state->wavePattCnts);

    return state;
}

int wfc_status(const wfc_State *state) {
    if (state == NULL) return wfc_callerError;

    return state->status;
}

int wfc_step(wfc_State *state) {
    if (state == NULL) return wfc_callerError;

    if (state->status != 0) return state->status;

    wfc__calcEntropies(
        state->pattCnt, state->patts, state->wave, state->modified,
        state->entropies);

    memset(state->modified.a, 0, WFC__A2D_SIZE(state->modified));

    int obsC0, obsC1;
    wfc__observeOne(
        state->ctx, state->pattCnt, state->patts, state->entropies,
        state->wave, state->modified,
        &obsC0, &obsC1);

    wfc__propagateFromSeed(
        state->ctx, state->n, state->options, state->pattCnt,
        obsC0, obsC1,
        state->overlaps, state->ripple, state->wave, state->modified);

    wfc__updateCnts(state->wave, state->modified,
        state->wavePattCnts, &state->collapsedCnt);
    state->status = wfc__calcStatus(state->pattCnt, state->wavePattCnts);

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

    for (int c0 = 0; c0 < state->dstD0; ++c0) {
        for (int c1 = 0; c1 < state->dstD1; ++c1) {
            int wC0, wC1, pC0, pC1;
            wfc__coordsDstToWave(c0, c1, state->wave, &wC0, &wC1, &pC0, &pC1);

            int patt = 0;
            for (int p = 0; p < state->pattCnt; ++p) {
                if (wfc__getBitA3d(state->wave, wC0, wC1, p)) {
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

    size_t pattsSz = (size_t)state->pattCnt * sizeof(*state->patts);
    clone->patts = (struct wfc__Pattern*)WFC_MALLOC(state->ctx, pattsSz);
    memcpy(clone->patts, state->patts, pattsSz);

    clone->overlaps.a = (unsigned*)WFC_MALLOC(
        state->ctx, WFC__A3D_SIZE(state->overlaps));
    memcpy(clone->overlaps.a, state->overlaps.a,
        WFC__A3D_SIZE(state->overlaps));

    clone->wave.a = (unsigned*)WFC_MALLOC(
        state->ctx, WFC__A3D_SIZE(state->wave));
    memcpy(clone->wave.a, state->wave.a,
        WFC__A3D_SIZE(state->wave));

    clone->wavePattCnts.a = (int*)WFC_MALLOC(
        state->ctx, WFC__A2D_SIZE(state->wavePattCnts));
    memcpy(clone->wavePattCnts.a, state->wavePattCnts.a,
        WFC__A2D_SIZE(state->wavePattCnts));

    clone->entropies.a = (float*)WFC_MALLOC(
        state->ctx, WFC__A2D_SIZE(state->entropies));
    memcpy(clone->entropies.a, state->entropies.a,
        WFC__A2D_SIZE(state->entropies));

    clone->modified.a = (uint8_t*)WFC_MALLOC(
        state->ctx, WFC__A2D_SIZE(state->modified));
    memcpy(clone->modified.a, state->modified.a,
        WFC__A2D_SIZE(state->modified));

    clone->ripple.a = (int*)WFC_MALLOC(
        state->ctx, WFC__A2D_SIZE(state->ripple));
    memcpy(clone->ripple.a, state->ripple.a,
        WFC__A2D_SIZE(state->ripple));

    return clone;
}

size_t wfc__sizeOfAllocs(wfc_State *state) {
    return
        (size_t)state->pattCnt * sizeof(*state->patts) +
        WFC__A3D_SIZE(state->overlaps) +
        WFC__A3D_SIZE(state->wave) +
        WFC__A2D_SIZE(state->wavePattCnts) +
        WFC__A2D_SIZE(state->entropies) +
        WFC__A2D_SIZE(state->modified) +
        WFC__A2D_SIZE(state->ripple);
}

void wfc_free(wfc_State *state) {
    if (state == NULL) return;

    void *ctx = state->ctx;
    (void)ctx;

    WFC_FREE(ctx, state->ripple.a);
    WFC_FREE(ctx, state->modified.a);
    WFC_FREE(ctx, state->entropies.a);
    WFC_FREE(ctx, state->wavePattCnts.a);
    WFC_FREE(ctx, state->wave.a);
    WFC_FREE(ctx, state->overlaps.a);
    WFC_FREE(ctx, state->patts);
    WFC_FREE(ctx, state);
}

int wfc_collapsedCount(const wfc_State *state) {
    if (state == NULL) return wfc_callerError;

    return state->collapsedCnt;
}

int wfc_patternCount(const wfc_State *state) {
    if (state == NULL) return wfc_callerError;

    return state->pattCnt;
}

int wfc_patternPresentAt(const wfc_State *state, int patt, int x, int y) {
    if (state == NULL ||
        patt < 0 || patt >= state->pattCnt ||
        x < 0 || x >= state->dstD1 ||
        y < 0 || y >= state->dstD0) {
        return wfc_callerError;
    }

    // x and y are only needed when some edges are fixed
    // and wave is smaller than destination.
    // wfc__coordsDstToWave handles all of that.
    // In that case, a different wave point
    // may contain which patterns are present for multiple destination points.
    int wC0, wC1;
    wfc__coordsDstToWave(y, x, state->wave, &wC0, &wC1, NULL, NULL);

    return wfc__getBitA3d(state->wave, wC0, wC1, patt);
}

int wfc_modifiedAt(const wfc_State *state, int x, int y) {
    if (state == NULL ||
        x < 0 || x >= state->dstD1 ||
        y < 0 || y >= state->dstD0) {
        return wfc_callerError;
    }

    int wC0, wC1;
    wfc__coordsDstToWave(y, x, state->wave, &wC0, &wC1, NULL, NULL);

    return WFC__A2D_GET(state->modified, wC0, wC1);
}

const unsigned char* wfc_pixelToBlitAt(
    const wfc_State *state, const unsigned char *src,
    int patt, int x, int y) {
    if (state == NULL ||
        src == NULL ||
        patt < 0 || patt >= state->pattCnt ||
        x < 0 || x >= state->dstD1 ||
        y < 0 || y >= state->dstD0) {
        return NULL;
    }

    struct wfc__A3d_cu8 srcA =
        {state->srcD0, state->srcD1, state->bytesPerPixel, src};

    // Similar case as in wfc_patternPresentAt(),
    // except here we care about the offsets
    // which are also the pattern-space coordinates of the pixel.
    int pC0, pC1;
    wfc__coordsDstToWave(y, x, state->wave, NULL, NULL, &pC0, &pC1);

    int sC0, sC1;
    wfc__coordsPattToSrc(
        state->n, state->patts[patt], pC0, pC1, srcA.d03, srcA.d13,
        &sC0, &sC1);

    return &WFC__A3D_GET(srcA, sC0, sC1, 0);
}

#endif // WFC_IMPLEMENTATION
