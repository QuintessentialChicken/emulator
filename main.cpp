#include <iostream>
#include <ostream>
#include <SDL3/SDL.h>

#include "chip8.h"
int PIXEL_SIZE = 20;
int SCREEN_WIDTH = 64;
int SCREEN_HEIGHT = 32;

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

    while (!emulator.stop_flag) {
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
