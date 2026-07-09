cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.3.0)
set(toolchainVersion 15_2_Rel1)
set(picotoolVersion 2.3.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2 CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(ouroboros-c-rp2350 C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

add_executable(ouroboros-c-rp2350
    src/button.c
    src/config.c
    src/delay.c
    src/dispatch.c
    src/input.c
    src/main.c
    src/ouroboros.c
    src/uart.c
    src/ws2812.c
)

pico_generate_pio_header(ouroboros-c-rp2350 ${CMAKE_CURRENT_LIST_DIR}/src/ws2812.pio)

target_include_directories(ouroboros-c-rp2350 PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/inc
)

target_link_libraries(ouroboros-c-rp2350
    pico_stdlib
    hardware_clocks
    hardware_gpio
    hardware_pio
    hardware_sync
    hardware_uart
    hardware_watchdog
)

# Match template defaults (USB stdio on, UART stdio off)
pico_enable_stdio_uart(ouroboros-c-rp2350 0)
pico_enable_stdio_usb(ouroboros-c-rp2350 1)

pico_add_extra_outputs(ouroboros-c-rp2350)
