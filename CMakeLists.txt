cmake_minimum_required(VERSION 3.29)
project(emulator)

set(CMAKE_CXX_STANDARD 26)

add_executable(emulator main.cpp
        chip8.cpp
        chip8.h)
