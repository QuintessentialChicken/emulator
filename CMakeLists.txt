cmake_minimum_required(VERSION 3.29)
project(emulator)

set(CMAKE_CXX_STANDARD 26)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)

add_executable(emulator main.cpp chip8.cpp chip8.h)

target_link_libraries(emulator PRIVATE SDL3::SDL3)
