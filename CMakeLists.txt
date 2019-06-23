cmake_minimum_required(VERSION 3.14)
project(chip8ler)

set(CMAKE_CXX_STANDARD 17)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(chip8ler main.cpp Chip8.cpp Chip8.hpp Display.cpp Display.hpp)
target_link_libraries(chip8ler ${SDL2_LIBRARIES})