// @TODO demo for sdl and stbi to call into libwfc

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <SDL.h>
#include <SDL_image.h>

#include "wfc.h"

const int screenW = 640, screenH = 480;

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

int loadAndWfcGenerateTextures(const char *path, int n, int dstW, int dstH,
    SDL_Renderer *renderer, SDL_Texture **texSrc, SDL_Texture **texDst) {
    int ret = 0;

    SDL_Surface *srcSurface = NULL;
    void *dstPixels = NULL;
    SDL_Surface *dstSurface = NULL;

    srcSurface = IMG_Load(path);
    if (srcSurface == NULL) {
        logError(IMG_GetError());
        ret = 1;
        goto cleanup;
    }

    {
        SDL_Surface *converted = SDL_ConvertSurfaceFormat(srcSurface,
            SDL_PIXELFORMAT_RGBA32, 0);
        if (converted == NULL) {
            logError(SDL_GetError());
            ret = 1;
            goto cleanup;
        }

        SDL_FreeSurface(srcSurface);
        srcSurface = converted;
    }

    const int bytesPerPixel = srcSurface->format->BytesPerPixel;
    const int srcW = srcSurface->w, srcH = srcSurface->h;

    assert(SDL_MUSTLOCK(srcSurface) == 0);

    dstPixels = malloc(dstW * dstH * bytesPerPixel);
    if (wfc_generate(
            n,
            srcW, srcH, srcSurface->pixels,
            dstW, dstH, dstPixels) != 0) {
        logError("WFC failed.");
        ret = 1;
        goto cleanup;
    }

    dstSurface = SDL_CreateRGBSurfaceWithFormatFrom(
        dstPixels, dstW, dstH,
        8 * bytesPerPixel, dstW * bytesPerPixel, srcSurface->format->format);
    if (dstSurface == NULL) {
        logError(SDL_GetError());
        ret = 1;
        goto cleanup;
    }

    *texSrc = SDL_CreateTextureFromSurface(renderer, srcSurface);
    if (*texSrc == NULL) {
        logError(SDL_GetError());
        ret = 1;
        goto cleanup;
    }

    *texDst = SDL_CreateTextureFromSurface(renderer, dstSurface);
    if (*texDst == NULL) {
        logError(SDL_GetError());
        ret = 1;
        goto cleanup;
    }

cleanup:
    if (dstSurface != NULL) SDL_FreeSurface(dstSurface);
    if (srcSurface != NULL) SDL_FreeSurface(srcSurface);
    free(dstPixels);

    return ret;
}

void renderTexture(SDL_Renderer *renderer, SDL_Texture *texture, int x, int y) {
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);

    SDL_RenderCopy(renderer, texture, NULL, &rect);
}

int main(int argc, char *argv[]) {
    int ret = 0;

    int calledSdlInit = 0;
    int calledImgInit = 0;
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Texture *texSrc = NULL;
    SDL_Texture *texDst = NULL;

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

    int sdlImgInitFlags = IMG_INIT_PNG;
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

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
        logError(SDL_GetError());
        ret = 1;
        goto cleanup;
    }

    srand((unsigned)time(NULL));

    if (loadAndWfcGenerateTextures(imagePath, wfcN, dstW, dstH,
            renderer, &texSrc, &texDst) != 0) {
        ret = 1;
        goto cleanup;
    }

    int texSrcW, texSrcH;
    SDL_QueryTexture(texSrc, NULL, NULL, &texSrcW, &texSrcH);
    int texDstH;
    SDL_QueryTexture(texDst, NULL, NULL, NULL, &texDstH);

    int quit = 0;
    while (!quit) {
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

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xff);
        SDL_RenderClear(renderer);

        renderTexture(renderer, texSrc, 0, (texDstH - texSrcH) / 2);
        renderTexture(renderer, texDst, texSrcW + 4, 0);

        SDL_RenderPresent(renderer);
    }

cleanup:
    if (texDst != NULL) SDL_DestroyTexture(texDst);
    if (texSrc != NULL) SDL_DestroyTexture(texSrc);
    if (renderer != NULL) SDL_DestroyRenderer(renderer);
    if (window != NULL) SDL_DestroyWindow(window);
    if (calledImgInit) IMG_Quit();
    if (calledSdlInit) SDL_Quit();

    return ret;
}
