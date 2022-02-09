add_executable(nepnes
  ui.cpp
  main.cpp
  $<TARGET_OBJECTS:imgui>
)

target_link_libraries(nepnes
  imgui
  SDL2
  OpenGL
  GLEW
  ${GTK3_LIBRARIES}
)

target_compile_definitions(nepnes
  PRIVATE NOC_FILE_DIALOG_GTK
)

target_include_directories(nepnes
  PRIVATE ${GTK3_INCLUDE_DIRS}
)
