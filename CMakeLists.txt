project(nepnes)

cmake_minimum_required(VERSION 3.15.4)

# Custom cmake modules
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Require C11 support.
set(CMAKE_C_STANDARD 11)

# Put all executables in build/.
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Set the appropriate compiler flags.
add_compile_options("$<$<CONFIG:DEBUG>:-Wall;-Wextra;-Wshadow;-Wno-unused-variable;-Wno-unused-function;-Wno-unused-parameter;-Werror;-Wstrict-overflow;-fno-strict-aliasing;-march=native>")
add_compile_options("$<$<CONFIG:RELEASE>:-Wall;-Wextra;-Wshadow;-Wpedantic;-Werror;-Wstrict-overflow;-fno-strict-aliasing;-march=native>")

# By default, include headers from src/.
include_directories(src)

find_package(PkgConfig REQUIRED)
find_package(Check REQUIRED)
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)

# TODO(ton): Need some platform specific check here.
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
pkg_check_modules(libzip REQUIRED IMPORTED_TARGET libzip)
pkg_check_modules(notcurses REQUIRED IMPORTED_TARGET notcurses)

add_subdirectory(src)
