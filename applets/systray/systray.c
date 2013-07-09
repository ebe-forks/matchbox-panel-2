/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/*
 * (C) 2006-2013 Intel Corp
 *
 * Author: Jorn Baayen <jorn@openedhand.com>
 *         Ross Burton <ross.burton@intel.com>
 *
 * Licensed under the GPL v2 or greater.
 */

#include <config.h>
#include <gtk/gtk.h>
#include <matchbox-panel/mb-panel.h>

#include "na-tray.h"

static void
on_realize (GtkWidget *widget, gpointer user_data)
{
  GdkScreen *screen;
  GtkWidget *tray;
  GtkOrientation orientation;

  screen = gtk_widget_get_screen (widget);

  /* Bit ugly but works to save passing a struct */
  orientation = GPOINTER_TO_INT (user_data);

  tray = (GtkWidget *)na_tray_new_for_screen (screen, orientation);

  gtk_widget_show (tray);

  gtk_container_add (GTK_CONTAINER (widget), tray);
}

G_MODULE_EXPORT GtkWidget *
mb_panel_applet_create (const char    *id,
                        GtkOrientation orientation)
{
        GtkWidget *box;

        box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

        gtk_widget_set_name (box, "MatchboxPanelSystemTray");

        g_signal_connect (box, "realize", G_CALLBACK (on_realize), GINT_TO_POINTER (orientation));

        gtk_widget_show (box);

        return box;
}
