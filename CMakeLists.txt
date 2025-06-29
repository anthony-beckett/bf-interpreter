cmake_minimum_required(VERSION 3.00)
project(Brainfsck_Interpreter C)

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_VERBOSE_MAKEFILE ON)

add_executable(
        bf
        main.c
        lib/stack.h
        lib/stack.c
)

target_compile_options(bf PRIVATE
        -Wall
        -Wextra
        -Werror
        -pedantic
        -fsanitize=address,undefined
)

target_link_options(bf PRIVATE
        -fsanitize=address,undefined
)
