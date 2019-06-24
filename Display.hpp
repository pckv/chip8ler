#ifndef CHIP8LER__DISPLAY_HPP_
#define CHIP8LER__DISPLAY_HPP_

#include <SDL.h>
#include "Chip8.hpp"

#define POINT_SIZE 10
#define SCREEN_WIDTH (DISPLAY_WIDTH * POINT_SIZE)
#define SCREEN_HEIGHT (DISPLAY_HEIGHT * POINT_SIZE)

class Display {
 public:
    explicit Display(Chip8 *chip_8);
    ~Display();

    void Draw();
    void HandleInput(bool &running);
 private:
    Chip8 *chip_8;
    SDL_Window *window;
    SDL_Renderer *renderer;

    uint8_t GetKeyIndex(SDL_Keycode keycode);
    // these are mappings to index 0-F. Layout should be:
    //      1   2   3   C
    //      4   5   6   D
    //      7   8   9   E
    //      A   0   B   F
    SDL_Keycode keymap[16] {
        SDLK_x, SDLK_1, SDLK_2, SDLK_3,
        SDLK_q, SDLK_w, SDLK_e, SDLK_a,
        SDLK_s, SDLK_d, SDLK_z, SDLK_c,
        SDLK_4, SDLK_r, SDLK_f, SDLK_v
    };
};

#endif //CHIP8LER__DISPLAY_HPP_
