#include <iostream>
#include <ostream>
#include <unordered_map>
#include <SDL3/SDL.h>

#include "chip8.h"
int PIXEL_SIZE = 20;
int SCREEN_WIDTH = 64;
int SCREEN_HEIGHT = 32;


//TODO Input is not working yet
std::unordered_map<SDL_Scancode, int> key_mapping = {
    { SDL_SCANCODE_1, 0x1 }, { SDL_SCANCODE_2, 0x2 }, { SDL_SCANCODE_3, 0x3 }, { SDL_SCANCODE_4, 0xC },
    { SDL_SCANCODE_Q, 0x4 }, { SDL_SCANCODE_W, 0x5 }, { SDL_SCANCODE_E, 0x6 }, { SDL_SCANCODE_R, 0xD },
    { SDL_SCANCODE_A, 0x7 }, { SDL_SCANCODE_S, 0x8 }, { SDL_SCANCODE_D, 0x9 }, { SDL_SCANCODE_F, 0xE },
    { SDL_SCANCODE_Z, 0xA }, { SDL_SCANCODE_X, 0x0 }, { SDL_SCANCODE_C, 0xB }, { SDL_SCANCODE_V, 0xF }
};

int main() {
    chip8 emulator;
    emulator.load_application("program.c8");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("Error: SDL_Init(): %s\n", SDL_GetError());
        return -1;
    }

    SDL_Window *window = SDL_CreateWindow(
        "An SDL3 window", // window title
        SCREEN_WIDTH * PIXEL_SIZE, // width, in pixels
        SCREEN_HEIGHT * PIXEL_SIZE, // height, in pixels
        SDL_WINDOW_OPENGL // flags - see below
    );

    SDL_Renderer *renderer = SDL_CreateRenderer(window, nullptr);
    SDL_FRect pixel{0, 0, static_cast<float>(PIXEL_SIZE), static_cast<float>(PIXEL_SIZE)};

    SDL_Event event;

    while (!emulator.stop_flag) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                emulator.stop_flag = true;
            }
            else if (event.type == SDL_EVENT_KEY_DOWN) {
                if (key_mapping.contains(event.key.scancode)) {
                    emulator.keys.at(key_mapping.at(event.key.scancode)) = 1;
                    std::cout << "Key down: " << key_mapping.at(event.key.scancode) << std::endl;
                }

                if (event.key.scancode == SDL_SCANCODE_ESCAPE) {
                    emulator.stop_flag = true;  // Close the program if ESC is pressed
                }
            }
            else if (event.type == SDL_EVENT_KEY_UP) {
                if (key_mapping.contains(event.key.scancode)) {
                    emulator.keys.at(key_mapping.at(event.key.scancode)) = 0;
                    std::cout << "Key up: " << key_mapping.at(event.key.scancode) << std::endl;
                }
            }
        }
        emulator.emulate_cycle();
        if (emulator.draw_flag) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            for (int y = 0; y < SCREEN_HEIGHT; y++) {
                for (int x = 0; x < SCREEN_WIDTH; x++) {
                    if (emulator.screen.at(y * SCREEN_WIDTH + x) != 0) {
                        pixel.x = static_cast<float>(x * PIXEL_SIZE);
                        pixel.y = static_cast<float>(y * PIXEL_SIZE);
                        SDL_RenderFillRect(renderer, &pixel);
                    }
                }
            }
            SDL_RenderPresent(renderer);
            emulator.draw_flag = false;
        }
        SDL_Delay(16);
    }

    // SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
