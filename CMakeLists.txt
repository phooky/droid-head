# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH "/home/phooky/tools/pico/pico-sdk")

set(BTSTACK_ROOT ${PICO_SDK_PATH}/lib/btstack)

set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.4.0")
  message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.4.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

project(droid_head C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(droid_head oled_pico.cpp ssd1306.cpp 
    font/font.cpp font/oled_font.cpp font/aurebesh.cpp)

pico_set_program_name(droid_head "droid_head")
pico_set_program_version(droid_head "0.1")

pico_enable_stdio_uart(droid_head 0)
pico_enable_stdio_usb(droid_head 1)

# Add the standard library to the build
target_link_libraries(droid_head
  pico_stdlib
  pico_btstack_ble
  pico_btstack_classic
  pico_btstack_cyw43
  pico_cyw43_arch_none
  hardware_spi
  hardware_gpio)


# Add the standard include files to the build
target_include_directories(droid_head PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts or any other standard includes, if required
)

pico_add_extra_outputs(droid_head)

