#include <iostream>
#include <string>
#include <thread>

#include "Chip8.hpp"
#include "Display.hpp"

#define CLOCK_SPEED 60

void draw_display_cout(Chip8 *chip_8) {
    for (auto & rows : chip_8->display) {
        for (uint8_t point : rows) {
            std::cout << (point ? "#" : " ");
        }
        std::cout << std::endl;
    }
}

int main(int argc, char **argv) {
    std::string rom_path = "/home/pc/dev/cpp/chip8ler/roms/programs/Chip8 emulator Logo [Garstyciuks].ch8";

    auto *chip_8 = new Chip8();
    chip_8->LoadRom(rom_path);

    auto *display = new Display();

    while (!chip_8->IsComplete()) {
        chip_8->Cycle();

        // Draw to display
        if (chip_8->ShouldUpdateDisplay()) {
            display->Draw(chip_8);
        }

        // TODO: implement buzz
        if (chip_8->ShouldBuzz()) {

        }

        // TODO: update keys

        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / CLOCK_SPEED));
    }

    delete(display);
    delete(chip_8);

    return 0;
}