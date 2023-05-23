#include <SDL.h>
#include <SDL_image.h>

#include "wfc.h"

void logError(const char *msg) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", msg);
}

SDL_Surface* loadSurface(const char *path) {
    SDL_Surface *surface = IMG_Load(path);
    if (surface == NULL) {
        logError(SDL_GetError());
    }
    return surface;
}

SDL_Texture* loadTexture(SDL_Renderer *renderer, const char *path) {
    SDL_Surface *surface = loadSurface(path);
    if (surface == NULL) return NULL;

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == NULL) {
        logError(SDL_GetError());
    }

    SDL_FreeSurface(surface);

    return texture;
}

void renderTexture(SDL_Renderer *renderer, SDL_Texture *texture, int x, int y) {
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);

    SDL_RenderCopy(renderer, texture, NULL, &rect);
}

// @TODO call wfc() with surface info, call SDL_CreateRGBSurfaceFrom to create another surface
int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    int ret = 0;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        logError(SDL_GetError());
        ret = 1;
        goto exit_sdl_init;
    }

    int sdlImgInitFlags = IMG_INIT_PNG;
    if ((IMG_Init(sdlImgInitFlags) & sdlImgInitFlags) != sdlImgInitFlags) {
        logError(SDL_GetError());
        ret = 1;
        goto exit_img_init;
    }

    SDL_Window *window = SDL_CreateWindow("Demo",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        640, 480,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (window == NULL) {
        logError(SDL_GetError());
        ret = 1;
        goto exit_sdl_create_window;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
        logError(SDL_GetError());
        ret = 1;
        goto exit_sdl_create_renderer;
    }

    SDL_Texture *texture = loadTexture(renderer, "samples/3Bricks.png");
    if (texture == NULL) {
        logError(SDL_GetError());
        ret = 1;
        goto exit_load_texture;
    }

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

        renderTexture(renderer, texture, 0, 0);

        SDL_RenderPresent(renderer);
    }

    if (texture != NULL) SDL_DestroyTexture(texture);
exit_load_texture:
    if (renderer != NULL) SDL_DestroyRenderer(renderer);
exit_sdl_create_renderer:
    if (window != NULL) SDL_DestroyWindow(window);
exit_sdl_create_window:
    IMG_Quit();
exit_img_init:
    SDL_Quit();
exit_sdl_init:

    return ret;
}
