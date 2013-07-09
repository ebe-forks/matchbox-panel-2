/*
 * (C) 2008 OpenedHand Ltd.
 *
 * Author: Ross Burton <ross@openedhand.com>
 *
 * Licensed under the GPL v2 or greater.
 */

#include <config.h>
#include <gtk/gtk.h>
#include <matchbox-panel/mb-panel.h>
#include "notify-store.h"
#include "mb-notification.h"

static void
reposition (GtkWindow *window)
{
  GtkRequisition req;
  GdkScreen *screen;

  screen = gtk_window_get_screen (window);

  gtk_widget_get_preferred_size (GTK_WIDGET (window), &req, NULL);

  if (req.height) {
    gtk_window_resize (window, req.width, req.height);
    /* TODO: get the primary monitor and then use
       gdk_screen_get_monitor_geometry() to get the geometry of the primary
       display, not the overall screen. */
    gtk_window_move (window,
                     gdk_screen_get_width (screen) - req.width,
                     gdk_screen_get_height (screen) - req.height);
    gtk_widget_show ((GtkWidget*)window);
  } else {
    gtk_widget_hide ((GtkWidget*)window);
  }
}

static gint
id_compare (gconstpointer a, gconstpointer b)
{
  MbNotification *notification = MB_NOTIFICATION (a);
  guint find_id = GPOINTER_TO_INT (b);
  return mb_notification_get_id (notification) - find_id;
}

static GtkWidget *
find_widget (GtkContainer *container, guint32 id)
{
  GList *children, *l;
  GtkWidget *w;

  children = gtk_container_get_children (container);
  l = g_list_find_custom (children, GINT_TO_POINTER (id), id_compare);
  w = l ? l->data : NULL;
  g_list_free (children);
  return w;
}

static void
on_closed (MbNotification *notification, MbNotifyStore *store)
{
  mb_notify_store_close (store, mb_notification_get_id (notification), ClosedDismissed);
}

static void
on_notification_added (MbNotifyStore *store, Notification *notification, GtkWindow *window)
{
  GtkWidget *box, *w;

  box  = gtk_bin_get_child (GTK_BIN (window));
  w = find_widget ((GtkContainer*)box, notification->id);
  if (!w) {
    w = mb_notification_new ();
    g_signal_connect (w, "closed", G_CALLBACK (on_closed), store);
    gtk_widget_show_all (w);
    gtk_box_pack_start (GTK_BOX (box), w, FALSE, FALSE, 0);
  }
  mb_notification_update (MB_NOTIFICATION (w), notification);

  reposition (window);
}

static void
on_notification_closed (MbNotifyStore *store, guint id, guint reason, GtkWindow *window)
{
  GtkWidget *box, *w;

  box  = gtk_bin_get_child (GTK_BIN (window));
  w = find_widget ((GtkContainer*)box, id);
  if (w)
    gtk_container_remove (GTK_CONTAINER (box), w);

  reposition (window);
}

G_MODULE_EXPORT GtkWidget *
mb_panel_applet_create (const char *id, GtkOrientation orientation)
{
  GtkWidget *window, *box;
  MbNotifyStore *notify;

  window = gtk_window_new (GTK_WINDOW_POPUP);
  gtk_widget_set_name (window, "MbNotificationBox");
  gtk_window_set_gravity (GTK_WINDOW (window), GDK_GRAVITY_SOUTH_EAST);

  box = gtk_vbox_new (TRUE, 0);
  gtk_container_add (GTK_CONTAINER (window), box);
  gtk_widget_show_all (window);

  notify = mb_notify_store_new ();
  g_signal_connect (notify, "notification-added", G_CALLBACK (on_notification_added), window);
  g_signal_connect (notify, "notification-closed", G_CALLBACK (on_notification_closed), window);

  reposition (GTK_WINDOW (window));

  return gtk_hbox_new (FALSE, 0);
}
