#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <SDL.h>

#include "stb_image.h"
#include "stb_image_write.h"

#include "util.h"
#define UNARGS_IMPLEMENTATION
#include "unargs.h"
#include "wfc.h"
#include "wfc_args.h"
#include "wfc_wrap.h"

const int screenW = 640, screenH = 480;
const Uint32 ticksPerFrame = 1000 / 60;

int main(int argc, char *argv[]) {
    int ret = 0;

    const int bytesPerPixel = 4;
    const Uint32 surfaceFormat = SDL_PIXELFORMAT_RGBA32;

    bool calledSdlInit = false;
    SDL_Window *window = NULL;
    unsigned char *srcPixels = NULL;
    SDL_Surface *surfaceSrc = NULL;
    SDL_Surface *surfaceDst = NULL;
    struct WfcWrapper wfc = {0};

    struct Args args;
    if (parseArgs(argc, argv, &args, false) != 0) {
        ret = 1;
        goto cleanup;
    }

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "%s\n", SDL_GetError());
        ret = 1;
        goto cleanup;
    }
    calledSdlInit = true;

    window = SDL_CreateWindow("libwfc - GUI",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        screenW, screenH,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (window == NULL) {
        fprintf(stderr, "%s\n", SDL_GetError());
        ret = 1;
        goto cleanup;
    }
    SDL_Surface *surfaceWin = SDL_GetWindowSurface(window);

    int srcW, srcH;
    srcPixels = stbi_load(args.pathIn, &srcW, &srcH, NULL, bytesPerPixel);
    if (srcPixels == NULL) {
        fprintf(stderr, "Error opening file %s: %s\n",
            args.pathIn, stbi_failure_reason());
        ret = 1;
        goto cleanup;
    }

    surfaceSrc = SDL_CreateRGBSurfaceWithFormatFrom(
        srcPixels, srcW, srcH,
        bytesPerPixel * 8, srcW * bytesPerPixel, surfaceFormat);
    if (surfaceSrc == NULL) {
        fprintf(stderr, "%s\n", SDL_GetError());
        ret = 1;
        goto cleanup;
    }
    assert(surfaceSrc->format->palette == NULL);
    assert(!SDL_MUSTLOCK(surfaceSrc));

    if (verifyArgs(args, srcW, srcH) < 0) {
        ret = 1;
        goto cleanup;
    }

    surfaceDst = SDL_CreateRGBSurfaceWithFormat(0,
        args.dstW, args.dstH, bytesPerPixel * 8, surfaceSrc->format->format);
    assert(surfaceDst->format->palette == NULL);
    assert(!SDL_MUSTLOCK(surfaceDst));

    srand(args.seed);
    int wfcOptions = argsToWfcOptions(args);

    if (wfcInit(
            args.n, wfcOptions, bytesPerPixel,
            srcW, srcH, surfaceSrc->pixels,
            args.dstW, args.dstH,
            &wfc) != 0) {
        fprintf(stderr, "WFC init failed.\n");
        ret = 1;
        goto cleanup;
    }
    bool wfcBlitComplete = false;

    const int scaleMin = 1, scaleMax = 8;
    int scale = 1;

    bool quit = false;
    while (!quit) {
        Uint32 ticksPrev = SDL_GetTicks();

        // input

        SDL_Event e;
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_WINDOWEVENT) {
                if (e.window.event == SDL_WINDOWEVENT_RESIZED) {
                    surfaceWin = SDL_GetWindowSurface(window);
                }
            } else if (e.type == SDL_KEYDOWN && e.key.repeat == 0) {
                if (e.key.keysym.sym == SDLK_ESCAPE) {
                    quit = true;
                } else if (e.key.keysym.sym == SDLK_KP_PLUS) {
                    if (scale * 2 <= scaleMax) scale *= 2;
                } else if (e.key.keysym.sym == SDLK_KP_MINUS) {
                    if (scale / 2 >= scaleMin) scale /= 2;
                }
            }
        }

        // update

        int status = wfcStep(&wfc);
        if (status < 0) {
            if (wfcBacktrack(&wfc) != 0) {
                fprintf(stderr, "WFC step failed.\n");
                ret = 1;
                goto cleanup;
            } else {
                fprintf(stdout, "WFC is backtracking.\n");
                wfcBlitAveraged(wfc, surfaceSrc->pixels,
                    args.dstW, args.dstH, surfaceDst->pixels);
            }
        } else if (status == 0) {
            wfcBlitAveraged(wfc, surfaceSrc->pixels,
                args.dstW, args.dstH, surfaceDst->pixels);
        } else if (!wfcBlitComplete) {
            wfcBlit(wfc, surfaceSrc->pixels, surfaceDst->pixels);
            wfcBlitComplete = true;
            fprintf(stdout, "WFC completed.\n");
        }

        // render

        SDL_FillRect(surfaceWin, NULL, SDL_MapRGB(surfaceWin->format, 0, 0, 0));

        SDL_BlitScaled(surfaceSrc, NULL, surfaceWin,
            &(SDL_Rect){
                0, (scale * args.dstH - scale * srcH) / 2,
                scale * srcW, scale * srcH});
        SDL_BlitScaled(surfaceDst, NULL, surfaceWin,
            &(SDL_Rect){
                scale * srcW + 4, 0,
                scale * args.dstW, scale * args.dstH});

        SDL_UpdateWindowSurface(window);

        Uint32 ticksCurr = SDL_GetTicks();
        if (ticksCurr - ticksPrev < ticksPerFrame) {
            SDL_Delay(ticksPerFrame - (ticksCurr - ticksPrev));
        }
    }

    if (wfcStatus(&wfc) > 0) {
        enum ImageFormat fmt = getImageFormat(args.pathOut);
        if (fmt == IMG_BMP) {
            if (stbi_write_bmp(args.pathOut,
                    args.dstW, args.dstH, bytesPerPixel,
                    surfaceDst->pixels) == 0) {
                fprintf(stderr, "Error writing to file %s\n", args.pathOut);
                ret = 1;
                goto cleanup;
            }
        } else if (fmt == IMG_PNG) {
            if (stbi_write_png(args.pathOut,
                    args.dstW, args.dstH, bytesPerPixel,
                    surfaceDst->pixels,
                    args.dstW * bytesPerPixel) == 0) {
                fprintf(stderr, "Error writing to file %s\n", args.pathOut);
                ret = 1;
                goto cleanup;
            }
        } else if (fmt == IMG_TGA) {
            if (stbi_write_tga(args.pathOut,
                    args.dstW, args.dstH, bytesPerPixel,
                    surfaceDst->pixels) == 0) {
                fprintf(stderr, "Error writing to file %s\n", args.pathOut);
                ret = 1;
                goto cleanup;
            }
        } else {
            assert(false);
        }
    }

cleanup:
    wfcFree(wfc);
    if (surfaceDst != NULL) SDL_FreeSurface(surfaceDst);
    if (surfaceSrc != NULL) SDL_FreeSurface(surfaceSrc);
    if (srcPixels != NULL) stbi_image_free(srcPixels);
    if (window != NULL) SDL_DestroyWindow(window);
    if (calledSdlInit) SDL_Quit();

    return ret;
}
