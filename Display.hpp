#ifndef CHIP8LER__DISPLAY_HPP_
#define CHIP8LER__DISPLAY_HPP_

#include <SDL.h>
#include "Chip8.hpp"

#define POINT_SIZE 10
#define SCREEN_WIDTH (DISPLAY_WIDTH * POINT_SIZE)
#define SCREEN_HEIGHT (DISPLAY_HEIGHT * POINT_SIZE)

class Display {
 public:
    Display();
    ~Display();

    void Draw(Chip8 *chip_8);
 private:
    SDL_Window *window;
    SDL_Renderer *renderer;
};

#endif //CHIP8LER__DISPLAY_HPP_
