add_executable(dbg
  dbg.c
  main.c
  options.c
  window.c
)

find_package(Curses REQUIRED)

target_link_libraries(dbg
  PRIVATE
  libnepnes
  ${CURSES_LIBRARIES}
)
