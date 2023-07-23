#include <assert.h>
#include <string.h>
#include <time.h>

#include <SDL.h>
#include <SDL_image.h>

#include "cmd_args.h"
#include "wfc_wrap.h"

const int screenW = 640, screenH = 480;
const Uint32 ticksPerFrame = 1000 / 60;

void logInfo(const char *msg) {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s\n", msg);
}

void logError(const char *msg) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", msg);
}

int main(int argc, char *argv[]) {
    int ret = 0;

    int calledSdlInit = 0;
    int calledImgInit = 0;
    SDL_Window *window = NULL;
    SDL_Surface *surfaceSrc = NULL;
    SDL_Surface *surfaceDst = NULL;
    struct WfcWrapper wfc = {0};

    struct Args args;
    if (parseArgs(argc, argv, &args) != 0) {
        logError("Invalid arguments.");
        ret = 1;
        goto cleanup;
    }

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        logError(SDL_GetError());
        ret = 1;
        goto cleanup;
    }
    calledSdlInit = 1;

    int sdlImgInitFlags = IMG_INIT_JPG | IMG_INIT_PNG;
    if ((IMG_Init(sdlImgInitFlags) & sdlImgInitFlags) != sdlImgInitFlags) {
        logError(SDL_GetError());
        ret = 1;
        goto cleanup;
    }
    calledImgInit = 1;

    window = SDL_CreateWindow("libwfc - GUI",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        screenW, screenH,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (window == NULL) {
        logError(SDL_GetError());
        ret = 1;
        goto cleanup;
    }
    SDL_Surface *surfaceWin = SDL_GetWindowSurface(window);

    srand((unsigned)time(NULL));

    surfaceSrc = IMG_Load(args.imagePath);
    if (surfaceSrc == NULL) {
        logError(IMG_GetError());
        ret = 1;
        goto cleanup;
    }
    {
        SDL_Surface *surfaceConv = SDL_ConvertSurfaceFormat(surfaceSrc,
            SDL_PIXELFORMAT_RGBA32, 0);
        if (surfaceConv == NULL) {
            logError(SDL_GetError());
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

    surfaceDst = SDL_CreateRGBSurfaceWithFormat(0,
        args.dstW, args.dstH, bytesPerPixel * 8, surfaceSrc->format->format);
    assert(surfaceDst->format->palette == NULL);
    assert(!SDL_MUSTLOCK(surfaceDst));

    if (wfcInit(
            args.wfcN, 0, bytesPerPixel,
            srcW, srcH, surfaceSrc->pixels,
            args.dstW, args.dstH,
            &wfc) != 0) {
        logError("WFC init failed.");
        ret = 1;
        goto cleanup;
    }
    int wfcBlitComplete = 0;

    const int scaleMin = 1, scaleMax = 8;
    int scale = 1;

    int quit = 0;
    while (!quit) {
        Uint32 ticksPrev = SDL_GetTicks();

        // input

        SDL_Event e;
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = 1;
            } else if (e.type == SDL_WINDOWEVENT) {
                if (e.window.event == SDL_WINDOWEVENT_RESIZED) {
                    surfaceWin = SDL_GetWindowSurface(window);
                }
            } else if (e.type == SDL_KEYDOWN && e.key.repeat == 0) {
                if (e.key.keysym.sym == SDLK_ESCAPE) {
                    quit = 1;
                } else if (e.key.keysym.sym == SDLK_KP_PLUS) {
                    if (scale * 2 <= scaleMax) scale *= 2;
                } else if (e.key.keysym.sym == SDLK_KP_MINUS) {
                    if (scale / 2 >= scaleMin) scale /= 2;
                }
            }
        }

        // update

        int status = wfcStep(wfc);
        if (status < 0) {
            logError("WFC step failed.");
            ret = 1;
            goto cleanup;
        } else if (status == 0) {
            wfcBlitAveraged(wfc,
                surfaceSrc->pixels, args.dstW, args.dstH, surfaceDst->pixels);
        } else if (!wfcBlitComplete) {
            wfcBlit(wfc, surfaceSrc->pixels, surfaceDst->pixels);
            wfcBlitComplete = 1;
            logInfo("WFC completed.");
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
