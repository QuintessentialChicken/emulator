#include "chip8.h"

#include <fstream>

#include <iostream>

#include <ostream>
#include <bits/fs_fwd.h>
#include <bits/fs_path.h>

chip8::chip8() : opcode{0}, memory{}, V{}, I{0}, pc{0x200}, sp{0}, key{} {
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

    switch (opcode & 0xF000) {
        case 0x0000:
            switch (opcode & 0x000F) {
                case 0x0000:
                    std::cout << "Clear screen" << std::endl;
                    break;
                case 0x000E:
                    std::cout << "Return from subroutine" << std::endl;
                    break;
                default:
                    std::cout << "Unknown opcode: " << opcode << std::endl;
                    break;
            }
            break;
        case 0x1000:
            std::cout << "Jump to address " << (opcode & 0x0FFF) << std::endl;
            break;
        case 0x2000:
            std::cout << "Call subroutine at " << (opcode & 0x0FFF) << std::endl;
            break;
        case 0x3000:
            std::cout << "Skip next instruction if V" << (opcode & 0x0F00) << " equals " << (opcode & 0x00FF) << std::endl;
            if (V.at(opcode & 0x0F00) == (opcode & 0x00FF)) pc += 2;
            break;
        case 0x4000:
            std::cout << "Skip next instruction if V" << (opcode & 0x0F00) << " does not equal " << (opcode & 0x00FF) << std::endl;
            if (V.at(opcode & 0x0F00) != (opcode & 0x00FF)) pc += 2;
            break;
        case 0x5000:
            std::cout << "Skip next instruction if V" << (opcode & 0x0F00) << " equals " << "V" << (opcode & 0x00F0) << std::endl;
            if (V.at(opcode & 0x0F00) == (V.at(opcode & 0x00F0))) pc += 2;
            break;
        case 0x6000:
            std::cout << "Set V" << (opcode & 0x0F00) << " to " << (opcode & 0x00FF) << std::endl;
            V.at((opcode & 0x0F00)) = opcode & 0x00FF;
            break;
        case 0x7000:
//TODO What happens on overflows?
            std::cout << "Add V" << (opcode & 0x00FF) << " to " << (opcode & 0x0F00) << std::endl;
            V.at(opcode & 0x0F00) += opcode & 0x00FF;
            break;
        case 0x8000:
            switch (opcode & 0x00F) {
                case 0x0000:
                    std::cout << "Set V" << (opcode & 0x0F00) << " to the value of " << "V" << (opcode & 0x00F0) << std::endl;
                    break;
                case 0x0001:
                    std::cout << "Set V" << (opcode & 0x0F00) << " to V" << (opcode & 0x0F00) << " OR " << (opcode & 0x00F0) << std::endl;
                    break;
                case 0x0002:
                    std::cout << "Set V" << (opcode & 0x0F00) << " to V" << (opcode & 0x0F00) << " AND " << (opcode & 0x00F0) << std::endl;
                    break;
                case 0x0003:
                    std::cout << "Set V" << (opcode & 0x0F00) << " to V" << (opcode & 0x0F00) << " XOR " << (opcode & 0x00F0) << std::endl;
                    break;
                case 0x0004:
                    std::cout << "Add V" << (opcode & 0x00F0) << " to V" << (opcode & 0x0F00) << ". VF is set to 1 for overflow and 0 for no overflow"  << std::endl;
                    break;
                case 0x0005:
                    std::cout << "Subtract V" << (opcode & 0x00F0) << " from V" << (opcode & 0x0F00) << ". Set VF to 0 for underflow and 0 for no underflow"  << std::endl;
                    break;
                case 0x0006:
                    std::cout << "Shift V" << (opcode & 0x0F00) << " to the right by 1, then store the least significant bit prior to the shift in VF"  << std::endl;
                    break;
                case 0x0007:
                    std::cout << "Set V" << (opcode & 0x0F00) << " to V" << (opcode & 0x00F0) << " - " << (opcode & 0x0F00) << ". VF is set to 0 for underflow and 0 for no underflow"  << std::endl;
                    break;
                case 0x000E:
                    std::cout << "Shift V" << (opcode & 0x0F00) << " to the left by 1. Set VF to 1 if most significant bit was set, 1 otherwise"  << std::endl;
                    break;
                default:
                    std::cout << "Unknown opcode: " << std::hex << opcode << std::endl;
            }
            break;
        case 0x9000:
            std::cout << "Skip next instruction if V" << (opcode & 0x0F00) << " does not equal " << "V" << (opcode & 0x0FFF) << std::endl;
            break;
        case 0xA000:
            std::cout << "Set I to address " << (opcode & 0x0FFF) << std::endl;
            break;
        case 0xB000:
            std::cout << "Jump to address " << (opcode & 0x0FFF) << " + V0" << std::endl;
            break;
        case 0xC000:
            std::cout << "Set V" << (opcode & 0x0F00) << " to the result of bitwise AND on random number and " << (opcode & 0x00FF) << std::endl;
            break;
        case 0xD000:
            std::cout << "Draw a sprite with width 8 and height " << (opcode & 0x000F) << " at (V" << (opcode & 0x0F00) << ", " << (opcode & 0x00F0) << ")" << std::endl;
            break;
        case 0xE000:
            switch (opcode & 0x000F) {
                case 0x000E:
                    std::cout << "Skip next instruction if key stored in V" << (opcode & 0x0F00) << "is pressed" << std::endl;
                    break;
                case 0x0001:
                    std::cout << "Skip next instruction if key stored in V" << (opcode & 0x0F00) << "is not pressed" << std::endl;
                    break;
                default:
                    std::cout << "Unknown opcode: " << std::hex << opcode << std::endl;
            }
            break;
        case 0xF000:
            switch (opcode & 0x00FF) {
                case 0x0007:
                    std::cout << "Set V" << (opcode & 0x0F00) << " to value of delay timer" << std::endl;
                    break;
                case 0x000A:
                    std::cout << "Await keypress, then store it in V" << (opcode & 0x0F00) << " (Blocking Operation)" << std::endl;
                    break;
                case 0x0015:
                    std::cout << "Set delay timer to V" << (opcode & 0x0F00) << std::endl;
                    break;
                case 0x0018:
                    std::cout << "Set sound timer to V" << (opcode & 0x0F00) << std::endl;
                    break;
                case 0x001E:
                    std::cout << "Add V" << (opcode & 0x0F00) << " to I. Does not affect VF" << std::endl;
                    break;
                case 0x0029:
                    std::cout << "Set I to the location of the sprite for the character in V" << (opcode & 0x0F00) << std::endl;
                    break;
                case 0x0033:
                    std::cout << "Store the binary-coded decimal representation of V" << (opcode & 0x0F00) << ", with the hundreds digit in memory at location in I, the tens digit at location I+1, and the ones digit at location I+2" << std::endl;
                    break;
                case 0x0055:
                    std::cout << "Store from V0 to V " << (opcode & 0x0F00) << " (including V" << (opcode & 0x0F00) << ") in memory, starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified." << std::endl;
                    break;
                case 0x0065:
                    std::cout << "Fill from V0 to V " << (opcode & 0x0F00) << " (including V" << (opcode & 0x0F00) << ") with values from memory, starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified." << std::endl;
                    break;
                default:
                    std::cout << "Unknown opcode: " << std::hex << opcode << std::endl;
            }
            break;
        default:
            std::cout << "Unknown opcode: " << std::hex << opcode << std::endl;
    }
    pc += 2;
}
