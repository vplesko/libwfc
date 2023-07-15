#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <SDL.h>
#include <SDL_image.h>

#include "wfc.h"

const int screenW = 640, screenH = 480;
const Uint32 ticksPerFrame = 1000 / 60;

void logError(const char *msg) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", msg);
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

    int calledSdlInit = 0;
    int calledImgInit = 0;
    SDL_Window *window = NULL;
    SDL_Surface *surfaceSrc = NULL;
    SDL_Surface *surfaceDst = NULL;

    const char *imagePath;
    int wfcN, dstW, dstH;
    if (parseArgs(argc, argv, &imagePath, &wfcN, &dstW, &dstH) != 0) {
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

    window = SDL_CreateWindow("Demo",
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

    surfaceSrc = IMG_Load(imagePath);
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
        dstW, dstH, bytesPerPixel * 8, surfaceSrc->format->format);
    assert(surfaceDst->format->palette == NULL);
    assert(!SDL_MUSTLOCK(surfaceDst));

    if (wfc_generate(wfcN, 0, bytesPerPixel,
        srcW, srcH, surfaceSrc->pixels,
        dstW, dstH, surfaceDst->pixels) != 0) {
        logError("WFC failed.");
        ret = 1;
        goto cleanup;
    }

    int quit = 0;
    while (!quit) {
        Uint32 ticksPrev = SDL_GetTicks();

        // input

        SDL_Event e;
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = 1;
            } else if (e.type == SDL_KEYDOWN && e.key.repeat == 0) {
                if (e.key.keysym.sym == SDLK_ESCAPE) quit = 1;
            }
        }

        // update

        // render

        SDL_BlitSurface(surfaceSrc, NULL, surfaceWin,
            &(SDL_Rect){0, (dstH - srcH) / 2, srcW, srcH});
        SDL_BlitSurface(surfaceDst, NULL, surfaceWin,
            &(SDL_Rect){srcW + 4, 0, dstW, dstH});

        SDL_UpdateWindowSurface(window);

        Uint32 ticksCurr = SDL_GetTicks();
        if (ticksCurr - ticksPrev < ticksPerFrame) {
            SDL_Delay(ticksPerFrame - (ticksCurr - ticksPrev));
        }
    }

cleanup:
    if (surfaceDst != NULL) SDL_FreeSurface(surfaceDst);
    if (surfaceSrc != NULL) SDL_FreeSurface(surfaceSrc);
    if (window != NULL) SDL_DestroyWindow(window);
    if (calledImgInit) IMG_Quit();
    if (calledSdlInit) SDL_Quit();

    return ret;
}
