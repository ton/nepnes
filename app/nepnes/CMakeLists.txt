add_executable(nepnes
  app.c
  app_window.c
  main.c
)

target_link_libraries(nepnes
  # OpenGL::GL
  PkgConfig::Gtk4
)

# install target
install(TARGETS nepnes)
install(CODE "execute_process(COMMAND ${XDG_DESKTOP_MENU_EXECUTABLE} install --novendor ${CMAKE_CURRENT_SOURCE_DIR}/assets/nepnes.desktop)")
install(CODE "execute_process(COMMAND ${XDG_ICON_RESOURCE_EXECUTABLE} install --novendor ${CMAKE_CURRENT_SOURCE_DIR}/assets/nepnes.png --size 64 nepnes)")

# uninstall target
add_custom_target(nepnes-uninstall-desktop-menu
  COMMAND ${XDG_DESKTOP_MENU_EXECUTABLE} uninstall nepnes.desktop
)

add_custom_target(nepnes-uninstall-icon
  COMMAND ${XDG_ICON_RESOURCE_EXECUTABLE} uninstall --size 64 nepnes
)

add_dependencies(uninstall nepnes-uninstall-desktop-menu nepnes-uninstall-icon)
