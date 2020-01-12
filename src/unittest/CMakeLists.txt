add_executable(nepnes_test
  main.c
  ines_test.c
  opcode_test.c
)

target_link_libraries(nepnes_test
  ${CHECK_LIBRARIES}
  libnepnes
)
