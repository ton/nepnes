add_subdirectory(nn/tools)
add_subdirectory(unittest)
add_subdirectory(vendor)

add_library(libnepnes
  nn/6502/cpu.c
  nn/6502/instruction.c
  nn/nes/mapper.c
  nn/nes/rom.c
  nn/std/io.c
  nn/std/util.c
  nn/std/flat_set.c
  nn/tools/da/da.c
)

target_link_libraries(libnepnes
  PRIVATE PkgConfig::libzip
)

add_executable(nepnes
  nn/ui.cpp
  nn/main.cpp
  $<TARGET_OBJECTS:imgui>
)

target_link_libraries(nepnes
  imgui
  SDL2
  OpenGL
  GLEW
  ${GTK3_LIBRARIES}
)

target_compile_definitions(nepnes
  PRIVATE NOC_FILE_DIALOG_GTK
)

target_include_directories(nepnes
  PRIVATE ${GTK3_INCLUDE_DIRS}
)
