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

#ifdef USE_LIBNOTIFY
  #include <libnotify/notify.h>
#endif

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

#ifdef USE_LIBNOTIFY
/* Notification closed */
static void
notification_closed_cb (NotifyNotification *n,
                        GtkWidget          *icon)
{
        GHashTable *hash;
        gpointer id;

        /* Remove reference to notification */
        hash = g_object_get_data (G_OBJECT (icon), "notification-hash");
        g_assert (hash);

        id = g_object_get_data (G_OBJECT (n), "id");

        g_hash_table_remove (hash, id);
}

/* Message sent */
static void
message_sent_cb (NaTrayManager      *manager,
                 GtkWidget           *icon,
                 const char          *message,
                 long                 id,
                 long                 timeout,
                 gpointer             user_data)
{
        GHashTable *hash;
        NotifyNotification *n;
        GError *error;

        /* Create notification */
        n = notify_notification_new (message,
                                     NULL,
                                     NULL,
                                     icon);
        notify_notification_set_timeout (n, timeout);

        error = NULL;
        if (!notify_notification_show (n, &error)) {
                g_warning (error->message);

                g_error_free (error);

                g_object_unref (n);

                return;
        }

        /* Keep a reference to the notification */
        hash = g_object_get_data (G_OBJECT (icon), "notification-hash");
        if (!hash) {
                hash = g_hash_table_new_full (g_int_hash,
                                              g_int_equal,
                                              NULL,
                                              (GDestroyNotify) g_object_unref);

                g_object_set_data_full (G_OBJECT (icon),
                                        "notification-hash",
                                        hash,
                                        (GDestroyNotify) g_hash_table_destroy);
        }

        g_hash_table_insert (hash, GINT_TO_POINTER (id), n);

        /* Make sure the notification is removed from the hash once
         * closed */
        g_object_set_data (G_OBJECT (n), "id", GINT_TO_POINTER (id));
        g_signal_connect (n,
                          "closed",
                          G_CALLBACK (notification_closed_cb),
                          icon);
}

/* Message cancelled */
static void
message_cancelled_cb (NaTrayManager *manager,
                      GtkWidget      *icon,
                      long            id,
                      gpointer        user_data)
{
        GHashTable *hash;
        NotifyNotification *n;
        GError *error;
        
        /* Look up reference to notification */
        hash = g_object_get_data (G_OBJECT (icon), "notification-hash");
        if (!hash)
                return;

        n = g_hash_table_lookup (hash, GINT_TO_POINTER (id));
        if (!n)
                return;

        /* Close it */
        error = NULL;
        notify_notification_close (n, &error);
        if (error) {
                g_warning (error->message);

                g_error_free (error);
        }
}
#endif

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

#ifdef USE_LIBNOTIFY
        if (!notify_is_initted ())
                notify_init ("matchbox-panel");
#endif

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

#ifdef USE_LIBNOTIFY
        g_signal_connect (manager,
                          "message-sent",
                          G_CALLBACK (message_sent_cb),
                          NULL);
        g_signal_connect (manager,
                          "message-cancelled",
                          G_CALLBACK (message_cancelled_cb),
                          NULL);
#endif

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
