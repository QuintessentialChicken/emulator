#include "chip8.h"

int main() {
    chip8 emulator;
    emulator.load_application("program.c8");
    while (!emulator.stop_flag) {
        emulator.emulate_cycle();
    }
    return 0;
}
