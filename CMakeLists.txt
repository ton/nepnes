project(nepnes)

cmake_minimum_required(VERSION 3.20)

# Custom cmake modules
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Require C11 support.
set(CMAKE_C_STANDARD 11)

# Put all executables in build/.
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# install target
include(GNUInstallDirs)
find_program(XDG_DESKTOP_MENU_EXECUTABLE xdg-desktop-menu)
find_program(XDG_ICON_RESOURCE_EXECUTABLE xdg-icon-resource)

# uninstall target
if(NOT TARGET uninstall)
  configure_file(
    "${CMAKE_MODULE_PATH}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

  add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()

# Set the appropriate compiler flags.
add_compile_options("$<$<CONFIG:DEBUG>:-Wall;-Wextra;-Wshadow;-Wno-unused-variable;-Wno-unused-function;-Wno-unused-parameter;-Werror;-Wstrict-overflow;-fno-strict-aliasing;-march=native>")
add_compile_options("$<$<CONFIG:RELEASE>:-Wall;-Wextra;-Wshadow;-Wpedantic;-Werror;-Wno-unused-parameter;-Wstrict-overflow;-fno-strict-aliasing;-march=native>")

# By default, include headers from the root directory.
include_directories(.)

find_package(Check REQUIRED)
find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)

# TODO(ton): Need some platform specific check here.
pkg_check_modules(Gtk4 REQUIRED IMPORTED_TARGET gtk4)
pkg_check_modules(libzip REQUIRED IMPORTED_TARGET libzip)
pkg_check_modules(notcurses notcurses>=3.0.4 REQUIRED IMPORTED_TARGET notcurses notcurses-core)

add_subdirectory(app)
add_subdirectory(lib)
add_subdirectory(unittest)
