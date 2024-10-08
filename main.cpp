#include <iostream>
#include <ostream>
#include <SDL3/SDL.h>

#include "chip8.h"
float PIXEL_SIZE = 10.0;

int main() {
    bool loopShouldStop = false;

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        printf("Error: SDL_Init(): %s\n", SDL_GetError());
        return -1;
    }

    SDL_Window *window = SDL_CreateWindow(
        "An SDL3 window", // window title
        640, // width, in pixels
        480, // height, in pixels
        SDL_WINDOW_OPENGL // flags - see below
    );

    if (window == nullptr) {
        // In the case that the window could not be made...
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create window: %s\n", SDL_GetError());
        return 1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);

    chip8 emulator;
    emulator.load_application("program.c8");

    //TODO Use proper array access
    while (!emulator.stop_flag) {
        emulator.emulate_cycle();
        if (emulator.draw_flag) {
            for (int y = 0; y < 32; ++y) {
                for (int x = 0; x < 64; ++x) {
                    if (emulator.screen[y][x] == 1) {  // If the pixel is "on"
                        SDL_FRect pixelRect;
                        pixelRect.x = static_cast<float>(x) * PIXEL_SIZE;  // Scale the pixel's position
                        pixelRect.y = static_cast<float>(y) * PIXEL_SIZE;
                        pixelRect.w = PIXEL_SIZE;      // Width of the scaled pixel
                        pixelRect.h = PIXEL_SIZE;      // Height of the scaled pixel
                        SDL_RenderFillRect(renderer, &pixelRect);  // Draw the filled rectangle
                    }
                }
            }
        }
    }

    //TODO How to sync emulator and display?
    while (!loopShouldStop)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_EVENT_QUIT:
                    loopShouldStop = true;
                    break;
                default:
                    loopShouldStop = true;
            }
        }
        SDL_RenderPresent(renderer);
        SDL_Delay(3000);
    }




    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}