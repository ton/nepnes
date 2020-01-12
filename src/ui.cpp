#include "ui.hpp"

#include "vendor/imgui/imgui.h"
#include <stdio.h>

void alsof::render_ui(Ui& ui)
{
  if (ImGui::BeginMainMenuBar())
  {
    if (ImGui::BeginMenu("File"))
    {
      ui.show_open_file_dialog = ImGui::MenuItem("Open", "Ctrl+O");
      if (ImGui::MenuItem("Quit", "Alt+F4"))
      {
      }
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }
}
