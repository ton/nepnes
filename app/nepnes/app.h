#ifndef NEPNES_APP_NEPNES_APP_H
#define NEPNES_APP_NEPNES_APP_H

#include <gtk/gtk.h>

#define NEPNES_APP_TYPE (nepnes_app_get_type())
G_DECLARE_FINAL_TYPE(NepnesApp, nepnes_app, NEPNES, APP, GtkApplication)

NepnesApp *nepnes_app_new(void);

#endif
