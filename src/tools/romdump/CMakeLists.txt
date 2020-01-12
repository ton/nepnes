add_executable(romdump
  romdump.c
  options.c
)

target_link_libraries(romdump
  libnepnes
)
