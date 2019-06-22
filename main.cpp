#include <iostream>
#include <string>
#include <thread>
#include "Chip8.hpp"

void draw_display(Chip8 *chip_8) {
    for (auto & y : chip_8->display) {
        for (unsigned char x : y) {
            std::cout << (x ? "#" : " ");
        }
        std::cout << std::endl;
    }
}

int main(int argc, char **argv) {
    int hz = 60;
    std::string rom_path = "/home/pc/dev/cpp/chip8ler/roms/programs/IBM Logo.ch8";

    auto *chip_8 = new Chip8();
    chip_8->LoadRom(rom_path);

    while (!chip_8->IsComplete()) {
        chip_8->Cycle();

        // Draw to the display
        if (chip_8->ShouldUpdateDisplay()) {
            draw_display(chip_8);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / hz));
    }

    return 0;
}