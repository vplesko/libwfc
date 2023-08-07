#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <SDL.h>

#include "stb_image.h"
#include "stb_image_write.h"

#define UNARGS_IMPLEMENTATION
#include "unargs.h"

#include "util.h"
#define WFC_IMPLEMENTATION
#include "wfc.h"
#include "wfc_args.h"
#include "wfc_wrap.h"

const int screenW = 640, screenH = 480;
const Uint32 ticksPerFrame = 1000 / 60;
const int scaleMin = 1, scaleMax = 8;

const char *instructions =
    "Controls:\n"
    "\t+      - Zoom in\n"
    "\t-      - Zoom out\n"
    "\tSpace  - Pause/unpause\n"
    "\tEscape - Quit";

struct GuiState {
    int scale;
    bool completed;
    bool paused;
};

void updateWindowTitle(struct GuiState guiState, SDL_Window *window) {
    enum { cap = 200 };
    char buff[cap];

    const char *status = "";
    if (guiState.completed) status = " (COMPLETED)";
    else if (guiState.paused) status = " (PAUSED)";

    int code = snprintf(buff, cap,
        "%s - %dx%s",
        "WFC GUI", guiState.scale, status);
    assert(code >= 0 && code + 1 <= cap);

    SDL_SetWindowTitle(window, buff);
}

void guiStateInitiate(struct GuiState *guiState, SDL_Window *window) {
    guiState->scale = 1;
    updateWindowTitle(*guiState, window);
}

void guiStateIncScale(struct GuiState *guiState, SDL_Window *window) {
    if (guiState->scale * 2 <= scaleMax) {
        guiState->scale *= 2;
        updateWindowTitle(*guiState, window);
    }
}

void guiStateDecScale(struct GuiState *guiState, SDL_Window *window) {
    if (guiState->scale / 2 >= scaleMin) {
        guiState->scale /= 2;
        updateWindowTitle(*guiState, window);
    }
}

void guiStateSetCompleted(struct GuiState *guiState, SDL_Window *window) {
    guiState->completed = true;
    guiState->paused = true;
    updateWindowTitle(*guiState, window);
}

void guiStateTogglePause(struct GuiState *guiState, SDL_Window *window) {
    if (guiState->completed) return;

    guiState->paused = !guiState->paused;
    updateWindowTitle(*guiState, window);
}

SDL_Rect* renderRectSrc(
    struct GuiState guiState,
    int srcW, int srcH,
    int dstH,
    SDL_Rect *rect) {
    *rect = (SDL_Rect){
        0, (guiState.scale * dstH - guiState.scale * srcH) / 2,
        guiState.scale * srcW, guiState.scale * srcH};
    return rect;
}

SDL_Rect* renderRectDst(
    struct GuiState guiState,
    int srcW,
    int dstW, int dstH,
    SDL_Rect *rect) {
    *rect = (SDL_Rect){
        guiState.scale * srcW + 4, 0,
        guiState.scale * dstW, guiState.scale * dstH};
    return rect;
}

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

    window = SDL_CreateWindow("",
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

    fprintf(stdout, "%s\n", instructions);

    struct GuiState guiState;
    guiStateInitiate(&guiState, window);

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
                    guiStateIncScale(&guiState, window);
                } else if (e.key.keysym.sym == SDLK_KP_MINUS) {
                    guiStateDecScale(&guiState, window);
                } else if (e.key.keysym.sym == SDLK_SPACE) {
                    guiStateTogglePause(&guiState, window);
                }
            }
        }

        // update

        if (!guiState.paused) {
            int status = wfcStep(&wfc);
            if (status == wfc_failed) {
                if (wfcBacktrack(&wfc) != 0) {
                    fprintf(stdout, "WFC failed.\n");
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
            } else {
                assert(status == wfc_completed);
                if (!guiState.completed) {
                    wfcBlit(wfc, surfaceSrc->pixels, surfaceDst->pixels);
                    guiStateSetCompleted(&guiState, window);
                    fprintf(stdout, "WFC completed.\n");
                }
            }
        }

        // render

        SDL_Rect rect;

        SDL_FillRect(surfaceWin, NULL, SDL_MapRGB(surfaceWin->format, 0, 0, 0));

        SDL_BlitScaled(surfaceSrc, NULL, surfaceWin,
            renderRectSrc(guiState, srcW, srcH, args.dstH, &rect));
        SDL_BlitScaled(surfaceDst, NULL, surfaceWin,
            renderRectDst(guiState, srcW, args.dstW, args.dstH, &rect));

        SDL_UpdateWindowSurface(window);

        Uint32 ticksCurr = SDL_GetTicks();
        if (ticksCurr - ticksPrev < ticksPerFrame) {
            SDL_Delay(ticksPerFrame - (ticksCurr - ticksPrev));
        }
    }

    if (args.pathOut != NULL) {
        if (wfcStatus(&wfc) == wfc_completed) {
            if (wfcWriteOut(&args, bytesPerPixel, surfaceDst->pixels) != 0) {
                ret = 1;
                goto cleanup;
            }
        } else {
            fprintf(stdout, "WFC has not completed, output not saved.\n");
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
