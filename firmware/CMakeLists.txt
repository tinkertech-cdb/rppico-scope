cmake_minimum_required(VERSION 3.13)

# Pico SDK 2.2.0 setup.
if(NOT DEFINED PICO_SDK_PATH)
    set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
endif()

if(NOT PICO_SDK_PATH)
    message(FATAL_ERROR "PICO_SDK_PATH is not set. Example: export PICO_SDK_PATH=$HOME/pico-sdk")
endif()

include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(rppico_scope_firmware C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(RPPICO_USB_VID 0xCAFE CACHE STRING "USB VID for development and test")
set(RPPICO_USB_PID 0x4010 CACHE STRING "USB PID for development and test")
set(RPPICO_USB_MANUFACTURER "OpenScope Community" CACHE STRING "USB manufacturer string")
set(RPPICO_USB_PRODUCT "XIAO RP2350 Scope+FG" CACHE STRING "USB product string")

# Default to RP2350 reference board. Override at configure time when a board
# definition for Seeed XIAO RP2350 is available in your SDK tree.
if(NOT DEFINED PICO_BOARD)
    set(PICO_BOARD pico2 CACHE STRING "Pico SDK board")
endif()

pico_sdk_init()

add_subdirectory(src)
