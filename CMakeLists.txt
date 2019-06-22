cmake_minimum_required(VERSION 3.14)
project(chip8ler)

set(CMAKE_CXX_STANDARD 17)

add_executable(chip8ler main.cpp Chip8.cpp Chip8.hpp)