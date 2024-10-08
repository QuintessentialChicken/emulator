#ifndef CHIP8_H

#define CHIP8_H
#include <array>
#include <stack>


class chip8 {
public:
    chip8();
    ~chip8() = default;
    // bool draw_flag;
    void emulate_cycle();
    bool load_application(const char * filename);
    bool stop_flag = false;
private:
    unsigned short opcode;
    std::array<unsigned char, 4096> memory;
    std::array<unsigned char, 16> V;
    unsigned short I;
    unsigned short pc;
    unsigned short sp;
    std::stack<unsigned short> stack;
    // unsigned char delay_timer;
    // unsigned char sound_timer;
    std::array<unsigned char, 16> key;
};
#endif //CHIP8_H
