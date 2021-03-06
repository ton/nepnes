add_executable(dbg
  assembly_pane.c
  breakpoints_pane.c
  cpu_pane.c
  debugger.c
  main.c
  nc.c
  options.c
  status_pane.c
)

target_link_libraries(dbg
  PRIVATE
  libnepnes
  notcurses-core
)
