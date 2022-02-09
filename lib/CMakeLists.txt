add_library(libnepnes
  6502/src/cpu.c
  6502/src/da.c
  6502/src/instruction.c
  nes/src/mapper.c
  nes/src/rom.c
  std/src/io.c
  std/src/util.c
  std/src/flat_set.c
)

# TODO(ton): for now, only one library for simplicity, can be split up in the
# future.
target_link_libraries(libnepnes
  PRIVATE PkgConfig::libzip
)
