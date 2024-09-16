cmake_minimum_required(VERSION 3.28)
project(code_and_samples C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(code_and_samples
        cbmp.c
        cbmp.h
        main.c
        minmax.h)
