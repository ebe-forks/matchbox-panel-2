/*
 * startup-monitor - A tray app that provides feedback on application startup.
 *
 * Copyright 2004 - 2007, Openedhand Ltd.
 * By Matthew Allum <mallum@o-hand.com>,
 * Stefan Schmidt <stefan@openmoko.org>,
 * Ross Burton <ross@openedhand.com>
 *
 * Originally based on mb-applet-startup-monitor
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <glib.h>
#include <gdk/gdkx.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>

#define SN_API_NOT_YET_FROZEN 1
#include <libsn/sn.h>
#include <libnotify/notify.h>

#include <string.h>

#define TIMEOUT 20

typedef struct LaunchItem {
  char *id;
  char *name;
  time_t when;
  guint timeout_id;
} LaunchItem;

typedef struct {
  GdkWindow *root_window;
  SnDisplay *sn_display;
  GList *launch_list;
  NotifyNotification *notify;
} StartupApplet;

static GdkFilterReturn filter_func (GdkXEvent *gdk_xevent,
                                    GdkEvent *event, StartupApplet *applet);

static gboolean timeout (StartupApplet *applet);

/* Destroy applet */
static void
startup_applet_free (StartupApplet *applet)
{
  gdk_window_remove_filter (applet->root_window,
                            (GdkFilterFunc) filter_func, applet);
  g_slice_free (StartupApplet, applet);
}

static void
update_notify (StartupApplet *applet)
{
  LaunchItem *item;
  char *msg;

  g_return_if_fail (applet->launch_list != NULL);

  item = applet->launch_list->data;

  msg = g_strdup_printf ("Starting %s...", item->name);

  if (applet->notify) {
    notify_notification_update (applet->notify, msg, NULL, NULL);
  } else {
    applet->notify = notify_notification_new (msg, NULL, NULL, NULL);
  }
  notify_notification_show (applet->notify, NULL);

  g_free (msg);
}

static void
hide_notify (StartupApplet *applet)
{
  if (applet->notify) {
    notify_notification_close (applet->notify, NULL);
    g_object_unref (applet->notify);
    applet->notify = NULL;
  }
}

static LaunchItem *
new_item (StartupApplet *applet, const char *id, const char *name)
{
  LaunchItem *item;

  g_return_val_if_fail (applet != NULL, NULL);
  g_return_val_if_fail (id != NULL, NULL);
  g_return_val_if_fail (name != NULL, NULL);
  
  item = g_new0 (LaunchItem, 1);
  item->id = g_strdup (id);
  item->name = g_strdup (name);
  
  item->when = time (NULL) + TIMEOUT;
  
  /* TODO: keep track of the earliest timeout and only wake up then */
#if GLIB_CHECK_VERSION(2, 14, 0)
  item->timeout_id = g_timeout_add_seconds (2, (GSourceFunc) timeout, applet);
#else
  item->timeout_id = g_timeout_add (2, (GSourceFunc) timeout, applet);
#endif 
  
  return item;
}

static void
free_item (LaunchItem *item)
{
  g_return_if_fail (item != NULL);
  
  g_source_remove (item->timeout_id);
  g_free (item->id);
  g_free (item->name);
  g_free (item);
}

static void
monitor_event_func (SnMonitorEvent *event, gpointer user_data)
{
  SnStartupSequence *sequence;
  const char *id, *name;
  StartupApplet *applet = (StartupApplet *) user_data;
  
  sequence = sn_monitor_event_get_startup_sequence (event);
  id = sn_startup_sequence_get_id (sequence);
  name = sn_startup_sequence_get_name (sequence);
  
  switch (sn_monitor_event_get_type (event)) {
  case SN_MONITOR_EVENT_INITIATED:
    {
      LaunchItem *item;
      item = new_item (applet, id, name);
      applet->launch_list = g_list_prepend (applet->launch_list, item);
      update_notify (applet);
    }
    break;

  case SN_MONITOR_EVENT_COMPLETED:
  case SN_MONITOR_EVENT_CANCELED:
    {
      GList *l;
      
      for (l = applet->launch_list; l; l = l->next) {
        LaunchItem *item = l->data;
        if (!strcmp (item->id, id)) {
          applet->launch_list = g_list_delete_link (applet->launch_list, l);
          free_item (item);
          break;
        }
      }
      
      if (applet->launch_list)
        update_notify (applet);
      else
        hide_notify (applet);
    }
    break;

  case SN_MONITOR_EVENT_CHANGED:
    /* TODO */
    break;
  }
}

static gboolean
timeout (StartupApplet *applet)
{
  time_t t;
  GList *l;
  
  if (!applet->notify)
    return TRUE;
  
  t = time (NULL);
  
  /* handle launchee timeouts */
  for (l = applet->launch_list; l; l = l->next) {
    LaunchItem *item = l->data;
    if ((item->when - t) <= 0) {
      applet->launch_list = g_list_delete_link (applet->launch_list, l);
      free_item (item);
      break;
    }
  }
  
  if (applet->launch_list) {
    update_notify (applet);
    return TRUE;
  } else {
    hide_notify (applet);
    return TRUE;
  }
  
  return TRUE;
}

static GdkFilterReturn
filter_func (GdkXEvent *gdk_xevent, GdkEvent *event, StartupApplet *applet)
{
  XEvent *xevent = (XEvent *) gdk_xevent;
  
  sn_display_process_event (applet->sn_display, xevent);
  
  return GDK_FILTER_CONTINUE;
}

G_MODULE_EXPORT GtkWidget *
mb_panel_applet_create (const char *id, GtkOrientation orientation)
{
  StartupApplet *applet;
  GtkWidget *widget;
  Display *xdisplay;
  SnMonitorContext *context;
  
  if (!notify_is_initted ())
    notify_init ("matchbox-panel");
  
  applet = g_slice_new0 (StartupApplet);
  
  widget = gtk_hbox_new (0, FALSE); /* grr */
  g_object_weak_ref (G_OBJECT (widget), (GWeakNotify)startup_applet_free, applet);
  
  xdisplay = GDK_DISPLAY_XDISPLAY (gtk_widget_get_display (widget));
  
  applet->sn_display = sn_display_new (xdisplay, NULL, NULL);
  
  context = sn_monitor_context_new (applet->sn_display,
                                    DefaultScreen (xdisplay),
                                    monitor_event_func,
                                    applet, NULL);
  
  /* We have to select for property events on at least one root window (but not
   * all as INITIATE messages go to all root windows)
   */
  XSelectInput (xdisplay, DefaultRootWindow (xdisplay), PropertyChangeMask);
  
  applet->root_window = gdk_window_lookup_for_display
    (gdk_x11_lookup_xdisplay (xdisplay), 0);
  
  gdk_window_add_filter (applet->root_window, (GdkFilterFunc)filter_func, applet);

  /* TODO: need to fix the panel to support invisible widgets */
  return widget;
}
