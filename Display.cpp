#include <iostream>

#include "Display.hpp"

#define err(msg) std::cout << msg << ": " << SDL_GetError() << std::endl

Display::Display(Chip8 *chip_8, const char *title) {
    this->chip_8 = chip_8;
    window = nullptr;
    renderer = nullptr;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO) < 0) {
        err("Failed to initialize SDL");
    }

    window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
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

void Display::Draw() {
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

uint8_t Display::GetKeyIndex(SDL_Keycode keycode) {
    for (int i = 0; i < sizeof(keymap); i++) {
        if (keycode == keymap[i]) {
            return i;
        }
    }

    return -1;
}

bool Display::HandleInput(bool &running) {
    SDL_Event e;

    while (SDL_PollEvent(&e) != 0) {
        // Handle quitting SDL
        if (e.type == SDL_QUIT) {
            running = false;
        }

        // Handle keypress
        if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) {
            uint8_t key = GetKeyIndex(e.key.keysym.sym);
            if (key >= 0) {
                chip_8->SetKey(key, e.type == SDL_KEYDOWN);
            }

            return true;
        }
    }

    return false;
}
