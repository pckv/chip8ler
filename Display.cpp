#include <iostream>

#include "Display.hpp"

#define err(msg) std::cout << msg << ": " << SDL_GetError() << std::endl

Display::Display() {
    window = nullptr;
    renderer = nullptr;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        err("Failed to initialize SDL");
    }

    window = SDL_CreateWindow("Chip8ler", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        err("Could not create SDL window");
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
        err("Could not create renderer");
    }

    // Give SDL some extra time to initialize in the system
    SDL_Delay(1000);
}

Display::~Display() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Display::Draw(Chip8 *chip_8) {
    SDL_SetRenderDrawColor(renderer, 0x0, 0x0, 0x0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, SDL_ALPHA_OPAQUE);

    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
            if (chip_8->display[y][x]) {
                SDL_Rect point = {x * POINT_SIZE, y * POINT_SIZE, POINT_SIZE, POINT_SIZE};
                SDL_RenderFillRect(renderer, &point);
            }
        }
    }

    SDL_RenderPresent(renderer);
}
