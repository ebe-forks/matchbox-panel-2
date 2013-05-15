/*
 * (C) 2013 Intel Corp
 *
 * Author: Ross Burton <ross.burton@intel.com>
 *
 * Licensed under the GPL v2 or greater.
 */

#include <config.h>
#include <gtk/gtk.h>
#include <matchbox-panel/mb-panel.h>

static void
on_clicked (GtkButton *button, gpointer user_data)
{
  GError *error = NULL;

  if (!g_spawn_command_line_async ("matchbox-remote -exit", &error)) {
    g_printerr ("Cannot ask Matchbox to exit: %s\n", error->message);
    g_error_free (error);
  }
}

G_MODULE_EXPORT GtkWidget *
mb_panel_applet_create (const char *id, GtkOrientation orientation)
{
  GtkWidget *button;

  button = gtk_button_new_from_stock (GTK_STOCK_QUIT);
  gtk_widget_set_name (button, "MatchboxPanelExit");
  g_signal_connect (button, "clicked", G_CALLBACK (on_clicked), NULL);
  gtk_widget_show (button);

  return button;
};
