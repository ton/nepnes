#include "app.h"

#include "app_window.h"

struct _NepnesApp
{
  GtkApplication parent;
};

/* will create nepnes_app_get_type and set nepnes_app_parent_class */
G_DEFINE_TYPE(NepnesApp, nepnes_app, GTK_TYPE_APPLICATION)

static void nepnes_app_init(NepnesApp *app)
{
}

static void nepnes_app_activate(GApplication *app)
{
  NepnesAppWindow *win = nepnes_app_window_new(NEPNES_APP(app));
  gtk_window_present(GTK_WINDOW(win));
}

static void nepnes_app_open(GApplication *app, GFile **files, int n_files, const char *hint)
{
  GList *windows = gtk_application_get_windows(GTK_APPLICATION(app));
  NepnesAppWindow *win =
      windows ? NEPNES_APP_WINDOW(windows->data) : nepnes_app_window_new(NEPNES_APP(app));

  for (int i = 0; i < n_files; i++) nepnes_app_window_open(win, files[i]);

  gtk_window_present(GTK_WINDOW(win));
}

static void nepnes_app_class_init(NepnesAppClass *class)
{
  G_APPLICATION_CLASS(class)->activate = nepnes_app_activate;
  G_APPLICATION_CLASS(class)->open = nepnes_app_open;
}

NepnesApp *nepnes_app_new(void)
{
  const GApplicationFlags flags = G_APPLICATION_HANDLES_OPEN;
  return g_object_new(
      NEPNES_APP_TYPE, "application-id", "com.tonvandenheuvel.nepnes", "flags", flags, NULL);
}
