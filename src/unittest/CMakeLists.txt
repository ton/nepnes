add_executable(nepnes_test
  da_test.c
  main.c
  rom_test.c
  opcode_test.c
)

target_link_libraries(nepnes_test
  PRIVATE ${CHECK_LIBRARIES}
  PRIVATE libnepnes
)
