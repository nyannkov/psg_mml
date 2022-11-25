cmake_minimum_required(VERSION 3.16)

include("$ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake")

project(pico_games C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if(CMAKE_BUILD_TYPE STREQUAL Debug)
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
endif()

pico_sdk_init()

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../../../../ psg_mml)

target_include_directories(psg_mml INTERFACE
    ./
    ${CMAKE_CURRENT_LIST_DIR}/../../../../inc
)

add_executable(${PSG_MML_PROJECT_NAME}
        ../../main.c
        ./board.c
        )

target_link_libraries(${PSG_MML_PROJECT_NAME}
            psg_mml
            pico_stdlib
            hardware_pwm
            hardware_irq
            )

add_compile_options(-Wall
        -Wno-format
        -Wno-unused-function
        -Wno-maybe-uninitialized
        )

pico_add_extra_outputs(${PSG_MML_PROJECT_NAME})

