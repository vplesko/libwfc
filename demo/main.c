#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <SDL.h>
#include <SDL_image.h>

#include "wfc.h"

const int screenW = 640, screenH = 480;

const char *image = "../samples/Angular.png";
const int wfcN = 3;
const int genW = 64, genH = 64;

void logError(const char *msg) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", msg);
}

int loadAndWfcGenerateTextures(const char *path, int n, int dstW, int dstH,
    SDL_Renderer *renderer, SDL_Texture **texLoaded, SDL_Texture **texGenerated) {
    int ret = 0;

    void *dstPixels = NULL;
    SDL_Surface *srcSurface = NULL;
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

    *texLoaded = SDL_CreateTextureFromSurface(renderer, srcSurface);
    if (*texLoaded == NULL) {
        logError(SDL_GetError());
        ret = 1;
        goto cleanup;
    }

    *texGenerated = SDL_CreateTextureFromSurface(renderer, dstSurface);
    if (*texGenerated == NULL) {
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
    (void)argc;
    (void)argv;

    int ret = 0;

    int calledSdlInit = 0;
    int calledImgInit = 0;
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Texture *texLoaded = NULL;
    SDL_Texture *texGenerated = NULL;

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

    srand(time(NULL));

    if (loadAndWfcGenerateTextures(image, wfcN, genW, genH,
            renderer, &texLoaded, &texGenerated) != 0) {
        ret = 1;
        goto cleanup;
    }

    int texLoadedW, texLoadedH;
    SDL_QueryTexture(texLoaded, NULL, NULL, &texLoadedW, &texLoadedH);
    int texGeneratedH;
    SDL_QueryTexture(texGenerated, NULL, NULL, NULL, &texGeneratedH);

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

        renderTexture(renderer, texLoaded, 0, (texGeneratedH - texLoadedH) / 2);
        renderTexture(renderer, texGenerated, texLoadedW + 4, 0);

        SDL_RenderPresent(renderer);
    }

cleanup:
    if (texGenerated != NULL) SDL_DestroyTexture(texGenerated);
    if (texLoaded != NULL) SDL_DestroyTexture(texLoaded);
    if (renderer != NULL) SDL_DestroyRenderer(renderer);
    if (window != NULL) SDL_DestroyWindow(window);
    if (calledImgInit) IMG_Quit();
    if (calledSdlInit) SDL_Quit();

    return ret;
}
