add_executable(romdump
  main.c
  options.c
)

target_link_libraries(romdump
  libnepnes
)
