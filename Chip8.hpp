#ifndef CHIP8LER__CHIP8_HPP_
#define CHIP8LER__CHIP8_HPP_

#define MEMORY_SIZE 4096
#define STACK_SIZE 16

#include <string>

class Chip8 {
 public:
    Chip8();
    ~Chip8();

    bool LoadRom(const std::string& path);
    void Cycle();
    bool IsComplete();

    void SetKey(uint8_t key, bool pressed);
 private:
    uint16_t opcode;
    uint8_t memory[MEMORY_SIZE] = {};
    uint16_t stack[STACK_SIZE] = {};

    uint8_t V[16] = {};  // general purpose registers
    uint16_t I;     // address register
    uint16_t pc;    // program counter
    uint8_t sp;     // stack pointer

    uint8_t dt;     // delay timer
    uint8_t st;     // sound timer

    uint8_t display[32][64] = {};

    // each bit represents a key
    uint16_t keys;
    bool GetKey(uint8_t key);

    void push(uint16_t value);
    uint16_t pop();
};

#endif //CHIP8LER__CHIP8_HPP_