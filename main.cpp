#include <iostream>
#include <string>
#include "Chip8.hpp"

int main(int argc, char **argv) {
    std::string rom_path = "/home/pc/dev/cpp/chip8ler/roms/programs/IBM Logo.ch8";

    auto *chip_8 = new Chip8();
    chip_8->LoadRom(rom_path);

    while (!chip_8->IsComplete()) {
        chip_8->Cycle();
    }

    return 0;
}