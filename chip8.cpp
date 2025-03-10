#include "chip8.h"

#include <filesystem>
#include <fstream>

#include <iostream>

#include <ostream>

unsigned char chip8_fontset[80] =
{
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80 // F
};

chip8::chip8()
    : draw_flag{true},
      screen{},
      keys{},
      opcode{0},
      memory{},
      V{},
      I{0},
      pc{0x200},
      delay_timer{},
      sound_timer{},
      rng{std::random_device{}()},
      dist{1, 255} {

    for (int i = 0; i < 80; ++i) {
        memory.at(i) = chip8_fontset[i];
    }
    // keys.at(0) = 1;
}

bool chip8::load_application(const char *filename) {
    std::ifstream input(filename, std::ios::binary);
    if (!input) {
        std::cout << "Failed to open file " << filename << std::endl;
        return false;
    }
    input.read(reinterpret_cast<std::istream::char_type *>(memory.data() + 512), static_cast<long long>(file_size(std::filesystem::path(filename))));
    return true;
}

void chip8::emulate_cycle() {
    if (pc >= 4096) {
        stop_flag = true;
        return;
    }
    opcode = memory[pc] << 8 | memory[pc + 1];
    std::cout << std::hex << opcode << ": ";

    switch (opcode & 0xF000) {
        case 0x0000:
            switch (opcode & 0x000F) {
                case 0x0000:
                    std::cout << "Clear screen" << std::endl;
                    for (int i = 0; i < 2048; ++i)
                        screen.at(i) = 0x0;
                    draw_flag = true;
                    pc += 2;
                    break;
                case 0x000E:
                    std::cout << "Return from subroutine" << std::endl;
                    pc = stack.top();
                    stack.pop();
                    pc += 2;
                    break;
                default:
                    std::cout << "Unknown opcode: " << opcode << std::endl;
                    break;
            }
            break;
        case 0x1000:
            std::cout << "Jump to address " << (opcode & 0x0FFF) << std::endl;
            pc = opcode & 0x0FFF;
            break;
        case 0x2000:
            std::cout << "Call subroutine at " << (opcode & 0x0FFF) << std::endl;
            stack.push(pc);
            pc = opcode & 0x0FFF;
            break;
        case 0x3000:
            std::cout << "Skip next instruction if V" << ((opcode & 0x0F00) >> 8) << " equals " << (opcode & 0x00FF) << std::endl;
            if (V.at((opcode & 0x0F00) >> 8) == (opcode & 0x00FF)) pc += 4;
            else pc += 2;
            break;
        case 0x4000:
            std::cout << "Skip next instruction if V" << ((opcode & 0x0F00) >> 8) << " does not equal " << (opcode & 0x00FF) << std::endl;
            if (V.at((opcode & 0x0F00) >> 8) != (opcode & 0x00FF)) pc += 4;
            else pc += 2;
            break;
        case 0x5000:
            std::cout << "Skip next instruction if V" << ((opcode & 0x0F00) >> 8) << " equals " << "V" << ((opcode & 0x00F0) >> 4) << std::endl;
            if (V.at((opcode & 0x0F00) >> 8) == V.at((opcode & 0x00F0) >> 4)) pc += 4;
            else pc += 2;
            break;
        case 0x6000:
            std::cout << "Set V" << ((opcode & 0x0F00) >> 8) << " to " << (opcode & 0x00FF) << std::endl;
            V.at((opcode & 0x0F00) >> 8) = opcode & 0x00FF;
            pc += 2;
            break;
        case 0x7000:
            std::cout << "Add V" << (opcode & 0x00FF) << " to " << ((opcode & 0x0F00) >> 8) << std::endl;
            V.at((opcode & 0x0F00) >> 8) += opcode & 0x00FF;
            pc += 2;
            break;
        case 0x8000:
            switch (opcode & 0x00F) {
                case 0x0000:
                    std::cout << "Set V" << ((opcode & 0x0F00) >> 8) << " to the value of " << "V" << ((opcode & 0x00F0) >> 4) << std::endl;
                    V.at((opcode & 0x0F00) >> 8) = (opcode & 0x00F0) >> 4;
                    pc += 2;
                    break;
                case 0x0001:
                    std::cout << "Set V" << ((opcode & 0x0F00) >> 8) << " to V" << ((opcode & 0x0F00) >> 8) << " OR V" << ((opcode & 0x00F0) >> 4) << std::endl;
                    V.at((opcode & 0x0F00) >> 8) |= (opcode & 0x00F0) >> 4;
                    pc += 2;
                    break;
                case 0x0002:
                    std::cout << "Set V" << ((opcode & 0x0F00) >> 8) << " to V" << ((opcode & 0x0F00) >> 8) << " AND V" << ((opcode & 0x0F00) >> 4) << std::endl;
                    V.at((opcode & 0x0F00) >> 8) &= V.at((opcode & 0x00F0) >> 4);
                    pc += 2;
                    break;
                case 0x0003:
                    std::cout << "Set V" << ((opcode & 0x0F00) >> 8) << " to V" << ((opcode & 0x0F00) >> 8) << " XOR V" << ((opcode & 0x0F00) >> 4) << std::endl;
                    V.at((opcode & 0x0F00) >> 8) ^= (opcode & 0x00F0) >> 4;
                    break;
                case 0x0004:
                    std::cout << "Add V" << ((opcode & 0x00F0) >> 4) << " to V" << ((opcode & 0x0F00) >> 8) << ". VF is set to 1 for overflow and 0 for no overflow" << std::endl;
                    if (V.at((opcode & 0x00F0) >> 4) > 0xFF - V.at((opcode & 0x0F00) >> 8)) {
                        V.at(0xF) = 1;
                    } else {
                        V.at(0xF) = 0;
                    }
                    V.at((opcode & 0x0F00) >> 8) += V.at((opcode & 0x00F0) >> 4);
                    pc += 2;
                    break;
                case 0x0005:
                    std::cout << "Subtract V" << ((opcode & 0x0F00) >> 4) << " from V" << ((opcode & 0x0F00) >> 8) << ". Set VF to 0 for underflow and 0 for no underflow" << std::endl;
                    if (V.at((opcode & 0x00F0) >> 4) > V.at((opcode & 0x0F00) >> 8)) {
                        V.at(0xF) = 0;
                    } else {
                        V.at(0xF) = 1;
                    }
                    V.at((opcode & 0x0F00) >> 8) -= V.at((opcode & 0x00F0) >> 4);
                    pc += 2;
                    break;
                case 0x0006:
                    std::cout << "Shift V" << ((opcode & 0x0F00) >> 8) << " to the right by 1, then store the least significant bit prior to the shift in VF" << std::endl;
                    V.at(0xF) = V.at((opcode & 0x0F00) >> 8) & 0x1;
                    V.at((opcode & 0x0F00) >> 8) >>= 1;
                    pc += 2;
                    break;
                case 0x0007:
                    std::cout << "Set V" << ((opcode & 0x0F00) >> 8) << " to V" << ((opcode & 0x00F0) >> 4) << " - " << ((opcode & 0x0F00) >> 8) << ". VF is set to 0 for underflow and 1 for no underflow" << std::endl;
                    if (V.at((opcode & 0x0F00) >> 8) > V.at((opcode & 0x0F00) >> 8)) {
                        V.at(0xF) = 0;
                    } else {
                        V.at(0xF) = 1;
                    }
                    V.at((opcode & 0x0F00) >> 8) = V.at((opcode & 0x00F0) >> 4) - V.at((opcode & 0x0F00) >> 8);
                    pc += 2;
                    break;
                case 0x000E:
                    std::cout << "Shift V" << ((opcode & 0x0F00) >> 8) << " to the left by 1. Set VF to 1 if most significant bit was set, 0 otherwise" << std::endl;
                    V.at(0xF) = V.at((opcode & 0x0F00) >> 8) >> 7;
                    V.at((opcode & 0x0F00) >> 8) <<= 1;
                    pc += 2;
                    break;
                default:
                    std::cout << "Unknown opcode: " << std::hex << opcode << std::endl;
            }
            break;
        case 0x9000:
            std::cout << "Skip next instruction if V" << ((opcode & 0x0F00) >> 8) << " does not equal " << "V" << ((opcode & 0x00F0) >> 4) << std::endl;
            if (V.at((opcode & 0x0F00) >> 8) != V.at((opcode & 0x00F0) >> 4)) pc += 4;
            else pc += 2;
            break;
        case 0xA000:
            std::cout << "Set I to address " << (opcode & 0x0FFF) << std::endl;
            I = opcode & 0x0FFF;
            pc += 2;
            break;
        case 0xB000:
            std::cout << "Jump to address " << (opcode & 0x0FFF) << " + V0" << std::endl;
            pc = (opcode & 0x0FFF) + V.at(0);
            break;
        case 0xC000:
            std::cout << "Set V" << ((opcode & 0x0F00) >> 8) << " to the result of bitwise AND on random number and " << (opcode & 0x00FF) << std::endl;
            V.at((opcode & 0x0F00) >> 8) = dist(rng) & (opcode & 0x00FF);
            pc += 2;
            break;
        case 0xD000: {
            std::cout << "Draw a sprite with width 8 and height " << (opcode & 0x000F) << " at V" << ((opcode & 0x0F00) >> 8) << ", V" << ((opcode & 0x00F0) >> 4) << ")" << std::endl;
            const unsigned short x = V.at((opcode & 0x0F00) >> 8);
            const unsigned short y = V.at((opcode & 0x00F0) >> 4);
            const unsigned short height = opcode & 0x000F;
            unsigned short row;
            V.at(0xF) = 0;
            // Iterate through each sprite row
            for (int yline = 0; yline < height; yline++) {
                // Fetch row (in the form of one unsigned short) from memory. Each bit in the short represents one pixel in the row
                row = memory[I + yline];
                // Iterate through each pixel in the row
                for (int xline = 0; xline < 8; xline++) {
                    // Check if the pixel is set (selecting the pixel with AND and 0x80 (one bit set to 1)). If not, we don't need to do anything
                    // Pixels are turned off by setting 1 on a pixel that's already 1
                    if ((row & (0x80 >> xline)) != 0) {
                        // Check if the corresponding pixel in memory is
                        if (screen.at((x + xline + ((y + yline) * 64))) == 1) {
                            V.at(0xF) = 1;
                        }
                        screen.at(x + xline + ((y + yline) * 64)) ^= 1;
                    }
                }
            }
            draw_flag = true;
            pc += 2;
        }
        break;
        case 0xE000:
            switch (opcode & 0x000F) {
                case 0x000E:
                    std::cout << "Skip next instruction if key stored in V" << ((opcode & 0x0F00) >> 8) << "is pressed" << std::endl;
                    if (keys.at(V.at((opcode & 0x0F00) >> 8) != 0)) {
                        pc += 4;
                    } else {
                        pc += 2;
                    }
                    break;
                case 0x0001:
                    std::cout << "Skip next instruction if key stored in V" << ((opcode & 0x0F00) >> 8) << "is not pressed" << std::endl;
                    if (keys.at(V.at((opcode & 0x0F00) >> 8)) == 0) {
                        std::cout << "Skipping, not pressed: " << std::hex << ((opcode & 0x0F00) >> 8) << std::endl;
                        pc += 4;
                    } else {
                        pc += 2;
                    }
                    break;
                default:
                    std::cout << "Unknown opcode: " << std::hex << opcode << std::endl;
            }
            break;
        case 0xF000:
            switch (opcode & 0x00FF) {
                case 0x0007:
                    std::cout << "Set V" << ((opcode & 0x0F00) >> 8) << " to value of delay timer" << std::endl;
                    V.at((opcode & 0x0F00) >> 8) = delay_timer;
                    pc += 2;
                    break;
                case 0x000A: {
                    std::cout << "Await keypress, then store it in V" << ((opcode & 0x0F00) >> 8) << " (Blocking Operation)" << std::endl;

                    bool key_pressed = false;
                    for (int i = 0; i < 16; i++) {
                        if (keys.at(i) != 0) {
                            V.at((opcode & 0x0F00) >> 8) = i;
                            key_pressed = true;
                        }
                    }
                    if (!key_pressed) return;
                    pc += 2;
                }
                break;
                case 0x0015:
                    std::cout << "Set delay timer to V" << ((opcode & 0x0F00) >> 8) << std::endl;
                    delay_timer = V.at((opcode & 0x0F00) >> 8);
                    pc += 2;
                    break;
                case 0x0018:
                    std::cout << "Set sound timer to V" << ((opcode & 0x0F00) >> 8) << std::endl;
                    sound_timer = V.at((opcode & 0x0F00) >> 8);
                    pc += 2;
                    break;
                case 0x001E:
                    std::cout << "Add V" << ((opcode & 0x0F00) >> 8) << " to I. Does not affect VF" << std::endl;
                // Set range overflow
                    if (I + V.at((opcode & 0x0F00) >> 8) > 0xFFF) V.at(0xF) = 1;
                    else V.at(0xF) = 0;
                    I += V.at((opcode & 0x0F00) >> 8);
                    pc += 2;
                    break;
                case 0x0029:
                    std::cout << "Set I to the location of the sprite for the character in V" << ((opcode & 0x0F00) >> 8) << std::endl;
                    I = V.at((opcode & 0x0F00) >> 8) * 0x5;
                    pc += 2;
                    break;
                case 0x0033:
                    std::cout << "Store the binary-coded decimal representation of V" << ((opcode & 0x0F00) >> 8) << ", with the hundreds digit in memory at location in I, the tens digit at location I+1, and the ones digit at location I+2" << std::endl;
                    memory.at(I) = V.at((opcode & 0x0F00) >> 8) / 100;
                    memory.at(I + 1) = V.at((opcode & 0x0F00) >> 8) / 10 % 10;
                    memory.at(I + 2) = V.at((opcode & 0x0F00) >> 8) % 100 % 10;
                    pc += 2;
                    break;
                case 0x0055:
                    std::cout << "Store from V0 to V " << ((opcode & 0x0F00) >> 8) << " (including V" << ((opcode & 0x0F00) >> 8) << ") in memory, starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified." << std::endl;
                    for (int i = 0; i < ((opcode & 0x0F00) >> 8) + 1; i++) {
                        memory.at(I + i) = V.at(i);
                    }
                // Set I to I + X + 1 according to original interpreter
                    I += (opcode & 0x0F00 >> 8) + 1;
                    pc += 2;
                    break;
                case 0x0065:
                    std::cout << "Fill from V0 to V " << ((opcode & 0x0F00) >> 8) << " (including V" << ((opcode & 0x0F00) >> 8) << ") with values from memory, starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified." << std::endl;
                    for (int i = 0; i < ((opcode & 0x0F00) >> 8) + 1; i++) {
                        V.at(i) = memory.at(I + i);
                    }
                    I += (opcode & 0x0F00 >> 8) + 1;
                    pc += 2;
                    break;
                default:
                    std::cout << "Unknown opcode: " << std::hex << opcode << std::endl;
            }
            break;
        default:
            std::cout << "Unknown opcode: " << std::hex << opcode << std::endl;
    }

    if (delay_timer > 0) delay_timer--;
    if (sound_timer > 0) sound_timer--;
}
