project(nepnes)

cmake_minimum_required(VERSION 3.15.4)

# Custom cmake modules
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Require C++17 support.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Put all executables in build/.
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Set the appropriate compiler flags.
add_compile_options("$<$<COMPILE_LANGUAGE:C>:-Wall;-Werror>")

# By default, include headers from src/.
include_directories(src)

find_package(Check REQUIRED)
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(libzip REQUIRED IMPORTED_TARGET libzip)

# TODO(ton): Need some platform specific check here.
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

add_subdirectory(src)
