#ifndef NEPNES_APP_NEPNES_APP_WINDOW_H
#define NEPNES_APP_NEPNES_APP_WINDOW_H

#include "app.h"

#include <gtk/gtk.h>

#define NEPNES_APP_WINDOW_TYPE (nepnes_app_window_get_type())
G_DECLARE_FINAL_TYPE(NepnesAppWindow, nepnes_app_window, NEPNES_APP, WINDOW, GtkApplicationWindow)

NepnesAppWindow *nepnes_app_window_new(NepnesApp *app);
void nepnes_app_window_open(NepnesAppWindow *win, GFile *file);

#endif
