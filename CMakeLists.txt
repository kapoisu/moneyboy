cmake_minimum_required(VERSION 3.22)
project(gameboy)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "-Wpedantic -Wall -Wconversion -Weffc++")

add_subdirectory(src)