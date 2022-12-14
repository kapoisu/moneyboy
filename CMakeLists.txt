cmake_minimum_required(VERSION 3.22)
project(gameboy)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "-Wpedantic -Wall -Wconversion -Weffc++")

find_package(SDL2 REQUIRED)
if(${SDL2_FOUND})
	message("SDL2 Dir = ${SDL2_DIR}")
	message("SDL2 Libraries = ${SDL2_LIBRARIES}")
	message("SDL2 Include = ${SDL2_INCLUDE_DIR}")
	message("SDL2 Bin = ${SDL2_BINDIR}")
endif()

add_subdirectory(src)
