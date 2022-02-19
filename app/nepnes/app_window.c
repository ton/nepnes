#include "app_window.h"

#include "app.h"

struct _NepnesAppWindow
{
  GtkApplicationWindow parent;
};

G_DEFINE_TYPE(NepnesAppWindow, nepnes_app_window, GTK_TYPE_APPLICATION_WINDOW)

static void nepnes_app_window_init(NepnesAppWindow *win)
{
  gtk_window_set_title(GTK_WINDOW(win), "nepnes");
  /* TODO(ton): sane default size; depends on NTSC/PAL resolution? */
  gtk_window_set_default_size(GTK_WINDOW(win), 400, 400);
}

static void nepnes_app_window_class_init(NepnesAppWindowClass *class)
{
}

NepnesAppWindow *nepnes_app_window_new(NepnesApp *app)
{
  return g_object_new(NEPNES_APP_WINDOW_TYPE, "application", app, NULL);
}

void nepnes_app_window_open(NepnesAppWindow *win, GFile *file)
{
}
