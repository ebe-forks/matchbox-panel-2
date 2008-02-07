/* 
 * (C) 2006 OpenedHand Ltd.
 *
 * Author: Jorn Baayen <jorn@openedhand.com>
 *
 * Licensed under the GPL v2 or greater.
 */

#include <config.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkvbox.h>
#include <matchbox-panel/mb-panel.h>

#include "na-tray-manager.h"

/* We need to force the redraw like this because gtk_widget_queue_draw ()
 * doesn't help.
 */

static void
force_icon_redraw (GtkWidget *box)
{
       gtk_widget_hide (box);
       gtk_widget_show (box);
}

static void
style_set_cb (GtkWidget *widget, 
              GtkStyle  *old_style, 
              gpointer  user_data)
{
        force_icon_redraw (widget);
}

/* Tray icon added */
static void
tray_icon_added_cb (NaTrayManager *manager,
                    GtkWidget      *icon,
                    GtkBox         *box)
{
        gtk_box_pack_start (box, icon, FALSE, FALSE, 0);
        force_icon_redraw (GTK_WIDGET (box));
}

/* Screen changed */
static void
screen_changed_cb (GtkWidget      *widget,
                   GdkScreen      *old_screen,
                   NaTrayManager *manager)
{
        GdkScreen *screen;

        screen = gtk_widget_get_screen (widget);
        if (na_tray_manager_check_running (screen)) {
                g_warning ("Another system tray manager is running. "
                           "Not managing screen.");

                return;
        }

        na_tray_manager_manage_screen (manager, screen);

        force_icon_redraw (widget);
}

G_MODULE_EXPORT GtkWidget *
mb_panel_applet_create (const char    *id,
                        GtkOrientation orientation)
{
        NaTrayManager *manager;
        GtkWidget *box;

        /* Is this a horizontal panel? */
        if (orientation == GTK_ORIENTATION_HORIZONTAL)
                box = gtk_hbox_new (0, FALSE);
        else
                box = gtk_vbox_new (0, FALSE);

        gtk_widget_set_name (box, "MatchboxPanelSystemTray");

        /* Create tray manager */
        manager = na_tray_manager_new ();
        na_tray_manager_set_orientation (manager, orientation);

        g_signal_connect (manager,
                          "tray-icon-added",
                          G_CALLBACK (tray_icon_added_cb),
                          box);

        g_signal_connect (box,
                          "screen-changed",
                          G_CALLBACK (screen_changed_cb),
                          manager);

        g_signal_connect (box,
                          "style-set",
                          G_CALLBACK (style_set_cb),
                          NULL);


        g_object_weak_ref (G_OBJECT (box),
                           (GWeakNotify) g_object_unref,
                           manager);
        
        /* Show! */
        gtk_widget_show (box);

        return box;
}
