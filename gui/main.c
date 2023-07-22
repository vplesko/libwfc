#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <SDL.h>
#include <SDL_image.h>

#include "wfc.h"

const int screenW = 640, screenH = 480;
const Uint32 ticksPerFrame = 1000 / 60;

void logInfo(const char *msg) {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s\n", msg);
}

void logError(const char *msg) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", msg);
}

// @TODO rand seed as arg
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

void wfcBlitAveraged(const wfc_State *wfc, int bytesPerPixel,
    const unsigned char *src,
    int dstW, int dstH, unsigned char *dst) {
    int pattCnt = wfc_patternCount(wfc);

    for (int j = 0; j < dstH; ++j) {
        for (int i = 0; i < dstW; ++i) {
            for (int b = 0; b < bytesPerPixel; ++b) {
                int sum = 0, cnt = 0;
                for (int p = 0; p < pattCnt; ++p) {
                    if (wfc_patternAvailable(wfc, p, i, j)) {
                        const unsigned char* px =
                            wfc_pixelToBlit(wfc, p, i, j, src);

                        sum += (int)px[b];
                        ++cnt;
                    }
                }

                unsigned char avg = (unsigned char)(sum / cnt);
                dst[j * dstW * bytesPerPixel + i * bytesPerPixel + b] = avg;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    int ret = 0;

    int calledSdlInit = 0;
    int calledImgInit = 0;
    SDL_Window *window = NULL;
    SDL_Surface *surfaceSrc = NULL;
    SDL_Surface *surfaceDst = NULL;
    wfc_State *wfc = NULL;

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

    wfc = wfc_init(wfcN, 0, bytesPerPixel,
        srcW, srcH, surfaceSrc->pixels,
        dstW, dstH);
    if (wfc == NULL) {
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

        int status = wfc_step(wfc);
        if (status < 0) {
            logError("WFC step failed.");
            ret = 1;
            goto cleanup;
        } else if (status == 0) {
            wfcBlitAveraged(wfc, bytesPerPixel,
                surfaceSrc->pixels, dstW, dstH, surfaceDst->pixels);
        } else if (!wfcBlitComplete) {
            wfc_blit(wfc, surfaceSrc->pixels, surfaceDst->pixels);
            wfcBlitComplete = 1;
            logInfo("WFC completed.");
        }

        // render

        SDL_FillRect(surfaceWin, NULL, SDL_MapRGB(surfaceWin->format, 0, 0, 0));

        SDL_BlitScaled(surfaceSrc, NULL, surfaceWin,
            &(SDL_Rect){
                0, (scale * dstH - scale * srcH) / 2,
                scale * srcW, scale * srcH});
        SDL_BlitScaled(surfaceDst, NULL, surfaceWin,
            &(SDL_Rect){
                scale * srcW + 4, 0,
                scale * dstW, scale * dstH});

        SDL_UpdateWindowSurface(window);

        Uint32 ticksCurr = SDL_GetTicks();
        if (ticksCurr - ticksPrev < ticksPerFrame) {
            SDL_Delay(ticksPerFrame - (ticksCurr - ticksPrev));
        }
    }

cleanup:
    if (wfc != NULL) wfc_free(wfc);
    if (surfaceDst != NULL) SDL_FreeSurface(surfaceDst);
    if (surfaceSrc != NULL) SDL_FreeSurface(surfaceSrc);
    if (window != NULL) SDL_DestroyWindow(window);
    if (calledImgInit) IMG_Quit();
    if (calledSdlInit) SDL_Quit();

    return ret;
}
