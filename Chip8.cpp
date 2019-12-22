#include <iostream>
#include <fstream>

#include "Chip8.hpp"

using Clock=std::chrono::system_clock;

#define LOG_V(n) "V" << int(n) << " (0x" << int(V[n]) << ")"
#define LOG_I "I (0x" << int(I) << ")"
#define LOG_DT "DT (0x" << int(dt) << ")"
#define LOG_ST "ST (0x" << int(st) << ")"
#define LOG_CHANGED std::endl << "\t→ "

// TODO: separate object construction from initialization
Chip8::Chip8(bool debug) {
    this->debug = debug;

    opcode = 0;
    I = 0;
    pc = 0x200;  // programs start at 0x200 (512)
    sp = 0;
    dt = 0;
    st = 0;

    keys = 0;  // every key is 0
    update_display = false;

    prev_timer_start = Clock::now();

    uint8_t font[] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0,  // 0
        0x20, 0x60, 0x20, 0x20, 0x70,  // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0,  // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0,  // 3
        0x90, 0x90, 0xF0, 0x10, 0x10,  // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0,  // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0,  // 6
        0xF0, 0x10, 0x20, 0x40, 0x40,  // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0,  // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0,  // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90,  // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0,  // B
        0xF0, 0x80, 0x80, 0x80, 0xF0,  // C
        0xE0, 0x90, 0x90, 0x90, 0xE0,  // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0,  // E
        0xF0, 0x80, 0xF0, 0x80, 0x80,  // F
    };

    for (int i = 0; i < sizeof(font); i++)
        memory[i] = font[i];

    // TODO: use proper seed
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
    uint8_t y = low >> 4 & 0x0F;
    uint8_t kk = low;

    // delay timer and sound timer
    auto now = Clock::now();
    long duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - prev_timer_start).count();

    if (duration > (1000 / 60)) {
        if (dt > 0) dt--;
        if (st > 0) st--;
        prev_timer_start = Clock::now();
    }

    update_display = false;

    if (debug) {
        std::cout << std::hex << std::uppercase;
        std::cout << "pc:0x" << pc << " opcode:0x" << opcode << " inst:";
    }

    switch (opcode & 0xF000) {
        case 0x0000:
            switch (low) {
                case 0xE0:  // CLS - Clear screen
                    if (debug) std::cout << "CLS";

                    for (auto & rows : display) {
                        for (uint8_t & point : rows) {
                            point = 0x0;
                        }
                    }
                    update_display = true;
                    pc += 2;
                    break;
                case 0xEE:  // RET - Return from call
                    if (debug) std::cout << "RET";

                    pc = stack[--sp] + 2;
                    break;
                default:
                    pc += 2;
            }
            break;
        case 0x1000:  // JP nnn
            if (debug) std::cout << "JP " << "0x" << nnn;
            pc = nnn;
            break;
        case 0x2000:  // CALL nnn
            if (debug) std::cout << "CALL " << "0x" << nnn;
            stack[sp++] = pc;
            pc = nnn;
            break;
        case 0x3000:  // SE Vx, kk
            if (debug) std::cout << "SE " LOG_V(x) << ", " << "0x" << int(kk);
            pc += V[x] == kk ? 4 : 2;
            break;
        case 0x4000:  // SNE Vx, kk
            if (debug) std::cout << "SNE " << LOG_V(x) << ", " << "0x" << int(kk);
            pc += V[x] != kk ? 4 : 2;
            break;
        case 0x5000:  // SE Vx, Vy
            if (debug) std::cout << "SE " << LOG_V(x) << ", " << LOG_V(y);
            pc += V[x] == V[y] ? 4 : 2;
            break;
        case 0x6000:  // LD Vx, kk
            if (debug) std::cout << "LD " << LOG_V(x) << ", " << "0x" << int(kk);
            V[x] = kk;
            pc += 2;
            if (debug) std::cout << LOG_CHANGED << LOG_V(x);
            break;
        case 0x7000:  // ADD Vx, kk
            if (debug) std::cout << "ADD " << LOG_V(x) << ", " << "0x" << int(kk);
            V[x] += kk;
            pc += 2;
            if (debug) std::cout << LOG_CHANGED << LOG_V(x);
            break;
        case 0x8000:
            switch (n) {
                case 0x0:  // LD Vx, Vy
                    if (debug) std::cout << "LD " << LOG_V(x) << ", " << LOG_V(y);
                    V[x] = V[y];
                    if (debug) std::cout << LOG_CHANGED << LOG_V(x);
                    break;
                case 0x1:  // OR Vx, Vy
                    if (debug) std::cout << "OR " << LOG_V(x) << ", " << LOG_V(y);
                    V[x] |= V[y];
                    if (debug) std::cout << LOG_CHANGED << LOG_V(x);
                    break;
                case 0x2:  // AND Vx, Vy
                    if (debug) std::cout << "AND " << LOG_V(x) << ", " << LOG_V(y);
                    V[x] &= V[y];
                    if (debug) std::cout << LOG_CHANGED << LOG_V(x);
                    break;
                case 0x3:  // XOR Vx, Vy
                    if (debug) std::cout << "XOR " << LOG_V(x) << ", " << LOG_V(y);
                    V[x] ^= V[y];
                    if (debug) std::cout << LOG_CHANGED << LOG_V(x);
                    break;
                case 0x4:  // ADD Vx, Vy
                    if (debug) std::cout << "ADD " << LOG_V(x) << ", " << LOG_V(y);
                    V[x] = V[x] + V[y];
                    V[0xF] = V[y] > V[x] ? 1 : 0;
                    if (debug) std::cout << LOG_CHANGED << LOG_V(x) << ", " << LOG_V(0xF);
                    break;
                case 0x5:  // SUB Vx, Vy
                    if (debug) std::cout << "SUB " << LOG_V(x) << ", " << LOG_V(y);
                    V[0xF] = V[y] > V[x] ? 0 : 1;
                    V[x] = V[x] - V[y];
                    if (debug) std::cout << LOG_CHANGED << LOG_V(x) << ", " << LOG_V(0xF);
                    break;
                case 0x6:  // SHR Vx, Vy
                    if (debug) std::cout << "SHR " << LOG_V(x) << ", " << LOG_V(y);
                    V[x] = V[y] >> 1;
                    V[0xF] = V[y] & 0x1u;
                    if (debug) std::cout << LOG_CHANGED << LOG_V(x) << ", " << LOG_V(0xF);

                    // from cowgod:
                    // V[0xF] = V[x] & 0x1;
                    // V[x] >>= 1;
                    break;
                case 0x7:  // SUBN Vx, Vy
                    if (debug) std::cout << "SUBN " << LOG_V(x) << ", " << LOG_V(y);
                    V[0xF] = V[y] < V[x] ? 0 : 1;
                    V[x] = V[y] - V[x];
                    if (debug) std::cout << LOG_CHANGED << LOG_V(x) << ", " << LOG_V(0xF);
                    break;
                case 0xE:  // SHL Vx, Vy
                    if (debug) std::cout << "SHL " << LOG_V(x) << ", " << LOG_V(y);
                    V[x] = V[y] << 1;
                    V[0xF] = V[y] >> 0x7 & 0x1;
                    if (debug) std::cout << LOG_CHANGED << LOG_V(x) << ", " << LOG_V(0xF);

                    // from cowgod:
                    // V[0xF] = (V[x] >> 7) & 0x1;
                    // V[x] <<= 1;
                    break;
            }

            pc += 2;
            break;
        case 0x9000:  // SNE Vx, Vy
            if (debug) std::cout << "SNE " << LOG_V(x) << ", " << LOG_V(y);
            pc += V[x] != V[y] ? 4 : 2;
            if (debug) std::cout << LOG_CHANGED << LOG_V(x);
            break;
        case 0xA000:  // LD I, nnn
            if (debug) std::cout << "LD " << LOG_I << ", " << "0x" << nnn;
            I = nnn;
            pc += 2;
            if (debug) std::cout << LOG_CHANGED << LOG_I;
            break;
        case 0xB000:  // JP V0, nnn
            if (debug) std::cout << "JP " << LOG_V(0) << ", " << "0x" << nnn;
            pc = nnn + V[0x0];
            break;
        case 0xC000:  // RND Vx, kk
            if (debug) std::cout << "RND " << LOG_V(x) << ", " << "0x" << int(kk);
            V[x] = (uint8_t) rand() & kk;
            pc += 2;
            if (debug) std::cout << LOG_CHANGED << LOG_V(x);
            break;
        case 0xD000:  // DRW Vx, Vy, n
            if (debug) std::cout << "DRW " << LOG_V(x) << ", " << LOG_V(y) << ", " << int(n);
            DrawSprite(V[x], V[y], n);
            update_display = true;
            pc += 2;
            break;
        case 0xE000:  // Input
            switch (low) {
                case 0x9E:  // SKP Vx
                    if (debug) std::cout << "SKP " << LOG_V(x);
                    pc += GetKey(V[x]) ? 4 : 2;
                    break;
                case 0xA1:  // SKNP Vx
                    if (debug) std::cout << "SKNP " << LOG_V(x);
                    pc += !GetKey(V[x]) ? 4 : 2;
                    break;
                default:
                    pc += 2;
            }
            break;
        case 0xF000:
            switch (low) {
                case 0x07:  // LD Vx, DT
                    if (debug) std::cout << "LD " << LOG_V(x) << ", " << LOG_DT;
                    V[x] = dt;
                    pc += 2;
                    if (debug) std::cout << LOG_CHANGED << LOG_V(x);
                    break;
                case 0x0A:  // LD Vx, K
                    if (debug) std::cout << "LD " << LOG_V(x) << ", K";
                    // do not increase program counter until a key is pressed
                    if (keys) {
                        int key;
                        for (key = 0; key < 0xF; key++) {
                            if (GetKey(key)) break;
                        }

                        V[x] = key;
                        pc += 2;
                        if (debug) std::cout << " (" << key << ")" << LOG_CHANGED << LOG_V(x);
                    }
                    break;
                case 0x15:  // LD DT, Vx
                    if (debug) std::cout << "LD " << LOG_DT << ", " << LOG_V(x);
                    dt = V[x];
                    pc += 2;
                    if (debug) std::cout << LOG_CHANGED << LOG_DT;
                    break;
                case 0x18:  // LD ST, Vx
                    if (debug) std::cout << "LD " << LOG_ST << ", " << LOG_V(x);
                    st = V[x];
                    pc += 2;
                    if (debug) std::cout << LOG_CHANGED << LOG_ST;
                    break;
                case 0x1E:  // ADD I, Vx
                    if (debug) std::cout << "ADD " << LOG_I << ", " << LOG_V(x);
                    V[0xF] = V[x] + I > 0xFFF ? 1 : 0;
                    pc += 2;
                    I += V[x];
                    if (debug) std::cout << LOG_CHANGED << LOG_I << ", " << LOG_V(0xF);
                    break;
                case 0x29:  // LD F, Vx
                    if (debug) std::cout << "LD F, " << LOG_V(x);
                    I = V[x] * 0x5;
                    pc += 2;
                    if (debug) std::cout << LOG_CHANGED << LOG_I;
                    break;
                case 0x33:  // LD B, Vx
                    if (debug) std::cout << "LD B, " << LOG_V(x);
                    memory[I] = V[x] / 100;      // hundredth digit
                    memory[I + 1] = (V[x] / 10) % 10;    // tenth digit
                    memory[I + 2] = V[x] % 10;          // ones digit
                    if (debug) std::cout << LOG_CHANGED << "MEMORY[" << LOG_I << "] (" << int(memory[I]) << ", " << int(memory[I + 1]) << ", " << int(memory[I + 2]) << ")";
                    pc += 2;
                    break;
                case 0x55:  // LD [I], Vx
                    if (debug) std::cout << "LD [I], V" << int(x) << LOG_CHANGED << "MEMORY[" << LOG_I << "] (";
                    for (int i = 0; i <= x; i++) {
                        memory[I + i] = V[i];
                        if (debug) std::cout << "0x" << int(memory[I + i]) << ", ";
                    }
                    I += x + 1;
                    pc += 2;
                    if (debug) std::cout << "), " << LOG_I;
                    break;
                case 0x65:  // LD Vx, [I]
                    if (debug) std::cout << "LD V" << int(x) << ", [I]" << LOG_CHANGED << "V..." << int(x) << " (";
                    for (int i = 0; i <= x; i++) {
                        V[i] = memory[I + i];
                        if (debug) std::cout << "0x" << int(V[i]) << ", ";
                    }
                    I += x + 1;
                    pc += 2;
                    if (debug) std::cout << "), " << LOG_I;
                    break;
                default:
                    pc += 2;
            }
            break;
        default:
            pc += 2;
    }

    if (debug) std::cout << std::endl;
}

void Chip8::DrawSprite(int posx, int posy, int height) {
    uint8_t x, y, point;

    // sprites drawn outside the frame should be wrapped back into it
    // NOTE: sprites drawn partially inside the frame should be clipped, not wrapped
    posx = posx % DISPLAY_WIDTH;
    posy = posy % DISPLAY_HEIGHT;
    V[0xF] = 0;

    for (int j = 0; j < height; j++) {
        y = posy + j;
        if (y >= DISPLAY_HEIGHT) continue;

        for (int i = 0; i < 8; i++) {
            x = posx + i;
            if (x >= DISPLAY_WIDTH) break;

            point = memory[I + j] >> (7 - i) & 0x1;
            if (debug) std::cout << LOG_CHANGED << "x: 0x" << int(x) << ", y: 0x" << int(y) << " point: 0x" << int(point)
                                 << " disp: 0x" << int(display[y][x]);
            display[y][x] ^= point;
            if (debug) std::cout << " → 0x" << int(display[y][x]);

            // catch bits that are changed by a set bit
            if (point && !display[y][x])
                V[0xF] = 1;
        }
    }

    if (debug) std::cout << LOG_CHANGED << LOG_V(0xF);
}

bool Chip8::HasNoInstructions() {
    return pc != 0x200 && opcode == 0x0000;
}

bool Chip8::ShouldUpdateDisplay() {
    return update_display;
}

bool Chip8::ShouldBuzz() {
    return st > 0;
}

bool Chip8::GetKey(uint8_t key) {
    return keys >> key & 0x1u;
}

void Chip8::SetKey(uint8_t key, bool pressed) {
    if (pressed)
        keys |= 0x1 << key;
    else
        keys &= ~(0x1 << key);
}
