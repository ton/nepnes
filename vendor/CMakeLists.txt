add_library(imgui OBJECT
  imgui/imgui.cpp
  imgui/imgui_demo.cpp
  imgui/imgui_draw.cpp
  imgui/imgui_tables.cpp
  imgui/imgui_widgets.cpp
  imgui/imgui_impl_opengl3.cpp
  imgui/imgui_impl_glfw.cpp
)

target_link_libraries(
  imgui
  PUBLIC
  dl
)
