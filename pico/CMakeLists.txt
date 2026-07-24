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
set(PICO_EXTRAS_FETCH_FROM_GIT ON)
set(PICO_BOARD pico CACHE STRING "Board type")

cmake_minimum_required(VERSION 3.13)
if(WIN32)
    include("${USERHOME}.pico-sdk/external/pico_sdk_import.cmake")
    include("${USERHOME}.pico-extras/external/pico_extras_import.cmake")
else()
    include("/opt/pico-sdk/external/pico_sdk_import.cmake")
    include("/opt/pico-extras/external/pico_extras_import.cmake")
endif()


 
project(prog C CXX ASM)
 
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
 
pico_sdk_init()

add_executable(${PROJECT_NAME} main.c avrprog.c)
pico_add_extra_outputs(${PROJECT_NAME})
 
target_link_libraries(${PROJECT_NAME} pico_stdlib hardware_spi)

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 1)
