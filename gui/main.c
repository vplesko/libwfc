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

const int screenW = 800, screenH = 600;
const Uint32 ticksPerFrame = 1000 / 60;
const int zoomMin = 1, zoomMax = 8;
const int cursorSizeMin = 1, cursorSizeMax = 5;

const char *instructions =
    "Controls:\n"
    "\t+       - Zoom in\n"
    "\t-       - Zoom out\n"
    "\tCtrl +  - Increase cursor size\n"
    "\tCtrl -  - Decrease cursor size\n"
    "\tSpace   - Pause/unpause\n"
    "\tEscape  - Quit";

bool isCtrlHeld(void) {
    const Uint8 *state = SDL_GetKeyboardState(NULL);
    return state[SDL_SCANCODE_LCTRL] || state[SDL_SCANCODE_RCTRL];
}

void incZoom(int *zoom) {
    if ((*zoom) * 2 <= zoomMax) (*zoom) *= 2;
}

void decZoom(int *zoom) {
    if ((*zoom) / 2 >= zoomMin) (*zoom) /= 2;
}

void incCursorSize(int *cursorSize) {
    if ((*cursorSize) + 2 <= cursorSizeMax) {
        (*cursorSize) += 2;
    }
}

void decCursorSize(int *cursorSize) {
    if ((*cursorSize) - 2 >= cursorSizeMin) {
        (*cursorSize) -= 2;
    }
}

enum GuiState {
    guiStateRunning,
    guiStatePaused,
    guiStateCompleted,
};

void updateWindowTitle(SDL_Window *window, enum GuiState guiState, int zoom) {
    enum { cap = 200 };
    char buff[cap];

    const char *status = "";
    if (guiState == guiStateCompleted) status = " (COMPLETED)";
    else if (guiState == guiStatePaused) status = " (PAUSED)";

    int code = snprintf(buff, cap,
        "%s - %dx%s",
        "WFC GUI", zoom, status);
    assert(code >= 0 && code + 1 <= cap);

    SDL_SetWindowTitle(window, buff);
}

SDL_Rect* renderRectSrc(
    int zoom, int srcW, int srcH, int dstH, SDL_Rect *rect) {
    *rect = (SDL_Rect){
        0, (zoom * dstH - zoom * srcH) / 2, zoom * srcW, zoom * srcH};
    return rect;
}

SDL_Rect* renderRectDst(
    int zoom, int srcW, int dstW, int dstH, SDL_Rect *rect) {
    *rect = (SDL_Rect){zoom * srcW + 2 * zoom, 0, zoom * dstW, zoom * dstH};
    return rect;
}

SDL_Rect* renderRectCursor(
    int zoom, int cursorSize, int srcW, int dstW, int dstH, SDL_Rect *rect) {
    SDL_Rect rectDst;
    renderRectDst(zoom, srcW, dstW, dstH, &rectDst);

    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    int centerX = (mouseX - rectDst.x) / zoom;
    int centerY = (mouseY - rectDst.y) / zoom;
    if (!between_i(centerX, 0, dstW - 1) ||
        !between_i(centerY, 0, dstH - 1)) {
        *rect = (SDL_Rect){0};
        return rect;
    }

    int pixelL = centerX - cursorSize / 2;
    int pixelT = centerY - cursorSize / 2;
    int pixelR = pixelL + cursorSize;
    int pixelB = pixelT + cursorSize;

    pixelL = max_i(0, pixelL);
    pixelT = max_i(0, pixelT);
    pixelR = min_i(dstW, pixelR);
    pixelB = min_i(dstH, pixelB);

    rect->x = rectDst.x + pixelL * zoom;
    rect->y = rectDst.y + pixelT * zoom;
    rect->w = (pixelR - pixelL) * zoom;
    rect->h = (pixelB - pixelT) * zoom;

    return rect;
}

void drawRect(SDL_Surface *surface, const SDL_Rect *rect, Uint32 color) {
    if (rect->w == 0 || rect->h == 0) return;

    SDL_Rect rects[] = {
        {rect->x, rect->y, rect->w, 1},
        {rect->x, rect->y, 1, rect->h},
        {rect->x, rect->y + rect->h - 1, rect->w, 1},
        {rect->x + rect->w - 1, rect->y, 1, rect->h},
    };
    SDL_FillRects(surface, rects, 4, color);
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

    enum GuiState guiState = guiStateRunning;
    int zoom = zoomMin;
    int cursorSize = cursorSizeMin;
    updateWindowTitle(window, guiState, zoom);

    bool quit = false;
    while (!quit) {
        Uint32 ticksPrev = SDL_GetTicks();

        bool pauseToggled = false;

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
                    if (isCtrlHeld()) {
                        incCursorSize(&cursorSize);
                    } else {
                        incZoom(&zoom);
                    }
                } else if (e.key.keysym.sym == SDLK_KP_MINUS) {
                    if (isCtrlHeld()) {
                        decCursorSize(&cursorSize);
                    } else {
                        decZoom(&zoom);
                    }
                } else if (e.key.keysym.sym == SDLK_SPACE) {
                    pauseToggled = true;
                }
            }
        }

        // update

        if (guiState == guiStateRunning) {
            if (pauseToggled) {
                guiState = guiStatePaused;
            } else {
                int status = wfcStep(&wfc);
                if (status == wfc_failed) {
                    if (wfcBacktrack(&wfc) != 0) {
                        fprintf(stdout, "WFC failed.\n");
                        ret = 1;
                        goto cleanup;
                    }
                    fprintf(stdout, "WFC is backtracking.\n");
                    wfcBlitAveraged(wfc, surfaceSrc->pixels,
                        args.dstW, args.dstH, surfaceDst->pixels);
                } else if (status == 0) {
                    wfcBlitAveraged(wfc, surfaceSrc->pixels,
                        args.dstW, args.dstH, surfaceDst->pixels);
                } else if (status == wfc_completed) {
                    wfcBlit(wfc, surfaceSrc->pixels, surfaceDst->pixels);
                    fprintf(stdout, "WFC completed.\n");

                    guiState = guiStateCompleted;
                } else {
                    assert(false);
                }
            }
        } else if (guiState == guiStatePaused) {
            if (pauseToggled) {
                guiState = guiStateRunning;
            }
        } else if (guiState == guiStateCompleted) {
            // do nothing
        } else {
            assert(false);
        }

        updateWindowTitle(window, guiState, zoom);

        // render

        SDL_Rect rect;

        SDL_FillRect(surfaceWin, NULL, SDL_MapRGB(surfaceWin->format, 0, 0, 0));

        SDL_BlitScaled(surfaceSrc, NULL, surfaceWin,
            renderRectSrc(zoom, srcW, srcH, args.dstH, &rect));
        SDL_BlitScaled(surfaceDst, NULL, surfaceWin,
            renderRectDst(zoom, srcW, args.dstW, args.dstH, &rect));
        // @TODO only draw when paused or completed
        drawRect(surfaceWin,
            renderRectCursor(
                zoom, cursorSize, srcW, args.dstW, args.dstH, &rect),
            SDL_MapRGBA(surfaceWin->format, 0x7f, 0x7f, 0x7f, 0));

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
