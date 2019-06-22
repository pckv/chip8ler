#include <iostream>
#include <fstream>

#include "Chip8.hpp"

// TODO: separate object construction from initialization
Chip8::Chip8() {
    opcode = 0;
    I = 0;
    pc = 0x200;  // programs start at 0x200 (512)
    sp = 0;
    dt = 0;
    st = 0;

    keys = 0;  // every key is 0

    // TODO: use better random seed
    srand(1234);
}

Chip8::~Chip8() = default;

bool Chip8::LoadRom(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to load rom: " << path << std::endl;
        return false;
    }

    int i = 0;
    while (!file.eof()) {
        memory[0x200 + i] = file.get();
        i++;
    }

    file.close();

    return true;
}

void Chip8::Cycle() {
    uint8_t high = memory[pc];
    uint8_t low = memory[pc + 1];
    opcode = high << 8 | low;

    uint16_t nnn = opcode & 0x0FFF;
    uint8_t n = low & 0x0F;
    uint8_t x = high & 0x0F;
    uint8_t y = low & 0xF0;
    uint8_t kk = low;

    std::cout << std::hex << std::uppercase;
    std::cout << "pc:0x" << pc << " opcode:0x" << opcode << std::endl;

    switch (opcode & 0xF000) {
        case 0x0000:
            switch (low) {
                case 0xE0:  // CLS - Clear screen
                    pc += 2;
                    break;
                case 0xEE:  // RET - Return from call
                    pc = stack[--sp] + 2;
                    break;
            }
            break;
        case 0x1000:  // JP nnn
            pc = nnn;
            break;
        case 0x2000:  // CALL nnn
            stack[sp++] = pc;
            pc = nnn;
            break;
        case 0x3000:  // SE Vx, kk
            pc += V[x] == kk ? 4 : 2;
            break;
        case 0x4000:  // SNE Vx, kk
            pc += V[x] != kk ? 4 : 2;
            break;
        case 0x5000:  // SE Vx, Vy
            pc += V[x] == V[y] ? 4 : 2;
            break;
        case 0x6000:  // LD Vx, kk
            V[x] = kk;
            pc += 2;
            break;
        case 0x7000:  // ADD Vx, kk
            V[x] += kk;
            pc += 2;
            break;
        case 0x8000:
            uint16_t result;

            switch (n) {
                case 0x0:  // LD Vx, Vy
                    V[x] = V[y];
                    break;
                case 0x1:  // OR Vx, Vy
                    V[x] = V[x] | V[y];
                    break;
                case 0x2:  // AND Vx, Vy
                    V[x] = V[x] & V[y];
                    break;
                case 0x3:  // XOR Vx, Vy
                    V[x] = V[x] ^ V[y];
                    break;
                case 0x4:  // ADD Vx, Vy
                    result = V[x] + V[y];
                    V[0xF] = result > 0xFF ? 1 : 0;
                    V[x] = result & 0x00FF;
                    break;
                case 0x5:  // SUB Vx, Vy
                    V[0xF] = V[x] > V[y] ? 1 : 0;
                    V[x] = V[x] - V[y];
                    break;
                case 0x6:  // SHR Vx
                    V[0xF] = V[x] & 0x1;
                    V[x] /= 2;
                    break;
                case 0x7:  // SUBN Vx, Vy
                    V[0xF] = V[y] > V[x] ? 1 : 0;
                    V[x] = V[y] - V[x];
                    break;
                case 0xE:  // SHL Vx
                    V[0xF] = V[x] & 0x1;
                    V[x] *= 2;
                    break;
            }

            pc += 2;
            break;
        case 0x9000:  // SNE Vx, Vy
            pc += V[x] != V[y] ? 4 : 2;
            break;
        case 0xA000:  // LD I, nnn
            I = nnn;
            pc += 2;
            break;
        case 0xB000:  // JP V0, nnn
            pc = nnn + V[0x0];
            break;
        case 0xC000:  // RND Vx, kk
            V[x] = rand() & kk;
            pc += 2;
            break;
        case 0xD000:  // DRW Vx, Vy, n
            // TODO: implement drawing
            pc += 2;
            break;
        case 0xE000:  // Input
            switch (low) {
                case 0x9E:  // SKP Vx
                    pc += GetKey(V[x]) ? 4 : 2;
                    break;
                case 0xA1:  // SKNP Vx
                    pc += !GetKey(V[x]) ? 4 : 2;
                    break;
                default:
                    pc += 2;
            }
            break;
        case 0xF000:
            switch (low) {
                case 0x07:  // LD Vx, DT
                    V[x] = dt;
                    pc += 2;
                    break;
                case 0x0A:  // LD Vx, K
                    // if any key is pressed
                    if (keys) {
                        int key;
                        for (key = 0; key < 0xF; key++)
                            if (GetKey(key))
                                break;

                        V[x] = key;
                        pc += 2;
                    }
                    break;
                case 0x15:  // LD DT, Vx
                    dt = V[x];
                    pc += 2;
                    break;
                case 0x18:  // LD ST, Vx
                    st = V[x];
                    pc += 2;
                    break;
                case 0x1E:  // ADD I, Vx
                    I = I + V[x];
                    pc += 2;
                    break;
                case 0x29:  // LD F, Vx
                    I = x * 5;
                    pc += 2;
                    break;
                case 0x33:  // LD B, Vx
                    memory[I] = V[x] / 100;             // hundredth digit
                    memory[I + 1] = V[x] % 100 / 10;    // tenth digit
                    memory[I + 2] = V[x] % 10;          // ones digit
                    pc += 2;
                    break;
                case 0x55:  // LD [I], Vx
                    for (int i = 0; i < x; i++)
                        memory[I + i] = V[i];
                    pc += 2;
                    break;
                case 0x65:  // LD Vx, [I]
                    for (int i = 0; i < x; i++)
                        V[i] = memory[I + i];
                    pc += 2;
                    break;
                default:
                    pc += 2;
            }
            break;
    }
}

bool Chip8::IsComplete() {
    return pc != 0x200 && opcode == 0x0000;
}

bool Chip8::GetKey(uint8_t key) {
    return keys >> key & 0x1;
}

void Chip8::SetKey(uint8_t key, bool pressed) {
    if (pressed)
        keys |= 0x1 << key;
    else
        keys &= ~(0x1 << key);
}

void Chip8::push(uint16_t value) {
    stack[sp++] = value;
}

uint16_t Chip8::pop() {
    return stack[--sp];
}
