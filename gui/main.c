#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <SDL.h>
#include <SDL_image.h>

#define UNARGS_IMPLEMENTATION
#include "unargs.h"

#include "wfc.h"
#include "wfc_args.h"
#include "wfc_wrap.h"

const int screenW = 640, screenH = 480;
const Uint32 ticksPerFrame = 1000 / 60;

int main(int argc, char *argv[]) {
    int ret = 0;

    bool calledSdlInit = false;
    bool calledImgInit = false;
    SDL_Window *window = NULL;
    SDL_Surface *surfaceSrc = NULL;
    SDL_Surface *surfaceDst = NULL;
    struct WfcWrapper wfc = {0};

    struct Args args;
    if (parseArgs(argc, argv, &args) != 0) {
        ret = 1;
        goto cleanup;
    }

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "%s\n", SDL_GetError());
        ret = 1;
        goto cleanup;
    }
    calledSdlInit = true;

    int sdlImgInitFlags = IMG_INIT_JPG | IMG_INIT_PNG;
    if ((IMG_Init(sdlImgInitFlags) & sdlImgInitFlags) != sdlImgInitFlags) {
        fprintf(stderr, "%s\n", SDL_GetError());
        ret = 1;
        goto cleanup;
    }
    calledImgInit = true;

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

    surfaceSrc = IMG_Load(args.inPath);
    if (surfaceSrc == NULL) {
        fprintf(stderr, "%s\n", IMG_GetError());
        ret = 1;
        goto cleanup;
    }
    {
        SDL_Surface *surfaceConv = SDL_ConvertSurfaceFormat(surfaceSrc,
            SDL_PIXELFORMAT_RGBA32, 0);
        if (surfaceConv == NULL) {
            fprintf(stderr, "%s\n", SDL_GetError());
            ret = 1;
            goto cleanup;
        }

        SDL_FreeSurface(surfaceSrc);
        surfaceSrc = surfaceConv;
    }
    assert(surfaceSrc->format->palette == NULL);
    assert(!SDL_MUSTLOCK(surfaceSrc));
    const int bytesPerPixel = surfaceSrc->format->BytesPerPixel;
    const int srcW = surfaceSrc->w, srcH = surfaceSrc->h;

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

cleanup:
    wfcFree(wfc);
    if (surfaceDst != NULL) SDL_FreeSurface(surfaceDst);
    if (surfaceSrc != NULL) SDL_FreeSurface(surfaceSrc);
    if (window != NULL) SDL_DestroyWindow(window);
    if (calledImgInit) IMG_Quit();
    if (calledSdlInit) SDL_Quit();

    return ret;
}
