#ifndef NEPNES_APP_NEPNES_UI_H
#define NEPNES_APP_NEPNES_UI_H

namespace nepnes
{
  /* UI state. */
  struct Ui
  {
    bool show_open_file_dialog{false};
    bool quit{false};
  };

  void render_ui(Ui& ui);
}

#endif
