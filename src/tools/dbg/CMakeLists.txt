add_executable(dbg
  dbg.c
  main.c
  options.c
)

target_link_libraries(dbg
  PRIVATE
  libnepnes
  notcurses
)
