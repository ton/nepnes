#ifndef NEPNES_UI_H
#define NEPNES_UI_H

namespace alsof
{
  /// UI state.
  struct Ui
  {
    bool show_open_file_dialog{false};
  };

  void render_ui(Ui& ui);
}

#endif
