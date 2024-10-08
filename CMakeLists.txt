cmake_minimum_required(VERSION 3.29)
project(emulator)

set(CMAKE_CXX_STANDARD 26)

set(SDL3_PATH "C:\\Users\\Lms\\Documents\\Programmieren\\SDL3-3.1.3")
include_directories(${SDL3_PATH}/include)
set(SDL3_LIB_DIR ${SDL3_PATH}/lib/x64)

add_executable(emulator main.cpp chip8.cpp chip8.h)

target_link_libraries(emulator ${SDL3_LIB_DIR}/SDL3.lib)
add_custom_command(TARGET emulator POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${SDL3_LIB_DIR}/SDL3.dll $<TARGET_FILE_DIR:emulator>)

