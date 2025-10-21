#include "../include/Game.h"
#include <SDL3/SDL.h>
#include <iostream>

void cleanup(SDL_Window *win) {
    SDL_DestroyWindow(win);
    SDL_Quit();
}

int main(int argc, char *argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "ERROR", "Error initializing SDL3", nullptr);
        return 1;
    };

    int width = 800;
    int height = 600;

    SDL_Window *win = SDL_CreateWindow("Star Defender", width, height, 0);
    if (!win) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "ERROR", "Error creating window", win);
        cleanup(win);
        return 1;
    }

    // Create and run the game
    Game game(win, width, height);
    game.run();

    cleanup(win);
    return 0;
}