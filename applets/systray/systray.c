/* 
 * (C) 2006 OpenedHand Ltd.
 *
 * Author: Jorn Baayen <jorn@openedhand.com>
 *
 * Licensed under the GPL v2 or greater.
 */

#include <gtk/gtkhbox.h>
#include <gtk/gtkvbox.h>
#include <matchbox-panel/mb-panel.h>

#include "eggtraymanager.h"

/* TODO: messages */

/* Tray icon added */
static void
tray_icon_added_cb (EggTrayManager *manager,
                    GtkWidget      *icon,
                    GtkBox         *box)
{
        gtk_box_pack_start (box, icon, FALSE, FALSE, 0);
}

/* Screen changed */
static void
screen_changed_cb (GtkWidget      *widget,
                   GdkScreen      *screen,
                   EggTrayManager *manager)
{
        if (!screen)
                screen = gdk_screen_get_default ();

        if (egg_tray_manager_check_running (screen)) {
                g_warning ("Another system tray manager is running. "
                           "Not managing screen.");

                return;
        }

        egg_tray_manager_manage_screen (manager, screen);
}

G_MODULE_EXPORT GtkWidget *
mb_panel_applet_create (const char *id,
                        int         panel_width,
                        int         panel_height)
{
        EggTrayManager *manager;
        GtkOrientation orientation;
        GtkWidget *box;

        /* Is this a vertical panel? */
        if (panel_width >= panel_height) {
                orientation = GTK_ORIENTATION_HORIZONTAL;

                box = gtk_hbox_new (0, FALSE);
        } else {
                orientation = GTK_ORIENTATION_VERTICAL;
                
                box = gtk_vbox_new (0, FALSE);
        }

        gtk_widget_set_name (box, "MatchboxPanelSystemTray");

        /* Create tray manager */
        manager = egg_tray_manager_new ();
        egg_tray_manager_set_orientation (manager, orientation);

        g_signal_connect (manager,
                          "tray-icon-added",
                          G_CALLBACK (tray_icon_added_cb),
                          box);

        g_signal_connect (box,
                          "screen-changed",
                          G_CALLBACK (screen_changed_cb),
                          manager);

        g_object_weak_ref (G_OBJECT (box),
                           (GWeakNotify) g_object_unref,
                           manager);
        
        /* Show! */
        gtk_widget_show (box);

        return box;
}
