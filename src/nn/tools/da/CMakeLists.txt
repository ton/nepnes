add_executable(da
  main.c
  options.c
)

target_link_libraries(da
  PRIVATE libnepnes
)
