#include <SDL.h>
#include <SDL_image.h>

void logError(const char *msg) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s\n", msg);
}

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

    int quit = 0;
    while (!quit) {
        // input

        SDL_Event e;
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = 1;
            }
        }

        // update

        // render

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xff);
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);
    }

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
