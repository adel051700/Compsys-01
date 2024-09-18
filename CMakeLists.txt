cmake_minimum_required(VERSION 3.28)
project(cellCounting C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(main
        cbmp.c
        cbmp.h
        main.c)

# Check if the operating system is MacOS
if(APPLE)
    # Set necessary flags here
    # Include necessary libraries here
endif()


