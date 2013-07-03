#include <gtk/gtk.h>

extern GtkWidget *mb_panel_applet_create (const char *id, GtkOrientation orientation);

int
main (int argc, char **argv)
{
  GtkWidget *applet;

  gtk_init (&argc, &argv);

  applet = mb_panel_applet_create ("arbitrary id", GTK_ORIENTATION_HORIZONTAL);

  g_object_ref_sink (applet);
  g_object_unref (applet);

  return 0;
}
