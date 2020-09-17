#include <iostream>
#include <string>
#include <thread>
#include <chrono>

#include "Chip8.hpp"
#include "Display.hpp"

#define CLOCK_SPEED 500 // CHIP8 should be 500Hz, SuperCHIP should be 1000hz

using Clock=std::chrono::system_clock;

void draw_display_cout(Chip8 *chip_8) {
    for (auto & rows : chip_8->display) {
        for (uint8_t point : rows) {
            std::cout << (point ? "#" : " ");
        }
        std::cout << std::endl;
    }
}

int main(int argc, char **argv) {
    bool running = true;

    if (argc < 2) {
        std::cout << "Usage: chip8ler <rom_file>" << std::endl;
        return 1;
    }

    std::string rom_path = argv[1];

    auto *chip_8 = new Chip8(true);
    if (!chip_8->LoadRom(rom_path)) {
        return 1;
    }

    std::cout << "Starting " << rom_path << std::endl;

    auto *display = new Display(chip_8, rom_path.c_str());

    std::chrono::system_clock::time_point stamp;

    while (running) {
        stamp = Clock::now();

        chip_8->Cycle();

        // Draw to display
        if (chip_8->ShouldUpdateDisplay()) {
            display->Draw();
        }

        // TODO: implement buzz
        if (chip_8->ShouldBuzz()) {

        }

        display->HandleInput(running);

        long delta = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - stamp).count();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / CLOCK_SPEED - delta));
    }

    delete(display);
    delete(chip_8);

    return 0;
}