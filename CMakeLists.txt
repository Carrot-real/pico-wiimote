cmake_minimum_required(VERSION 3.13)

include($ENV{PICO_SDK_PATH}/pico_sdk_init.cmake)

project(pico_wiimote C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(pico_wiimote main.c)

target_link_libraries(pico_wiimote pico_stdlib)

pico_enable_stdio_usb(pico_wiimote 1)
pico_enable_stdio_uart(pico_wiimote 0)

pico_add_extra_outputs(pico_wiimote)
