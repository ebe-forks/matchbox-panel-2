/*
 * startup-monitor - A tray app that provides feedback
 *                             on application startup.
 *
 * Copyright 2004 - 2007, Openedhand Ltd.
 * By Matthew Allum <mallum@o-hand.com>,
 * Stefan Schmidt <stefan@openmoko.org>
 *
 * Based on mb-applet-startup-monitor
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

#include <string.h>
#include <matchbox-panel/mb-panel.h>
#include <matchbox-panel/mb-panel-scaling-image.h>

#define TIMEOUT                   20
#define HOURGLASS_PIXMAPS 8

typedef struct LaunchItem {
        char *id;
        time_t when;
        guint timeout_id;
} LaunchItem;

typedef struct {
        MBPanelScalingImage *image;
        GdkWindow *root_window;
        SnDisplay *sn_display;
        SnMonitorContext *sn_context;
        GList *launch_list;
        gboolean hourglass_shown;
        int hourglass_cur_frame_n;
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
show_hourglass (StartupApplet *applet)
{
        gtk_widget_show (GTK_WIDGET (applet->image));
        applet->hourglass_shown = TRUE;
}

static void
hide_hourglass (StartupApplet *applet)
{
        gtk_widget_hide (GTK_WIDGET (applet->image));
        applet->hourglass_shown = FALSE;
}

static void
monitor_event_func (SnMonitorEvent *event, gpointer user_data)
{
        SnStartupSequence *sequence;
        const char *id;
        time_t t;
        StartupApplet *applet = (StartupApplet *) user_data;

        sequence = sn_monitor_event_get_startup_sequence (event);
        id = sn_startup_sequence_get_id (sequence);

        switch (sn_monitor_event_get_type (event)) {
        case SN_MONITOR_EVENT_INITIATED:
                {
                        /* Reset counter */
                        applet->hourglass_cur_frame_n = 0;

                        LaunchItem *item;
                        item = malloc (sizeof (LaunchItem));

                        /* Fillup list item with current launchee
                         * informations */
                        item->id = g_strdup (id);
                        t = time (NULL);
                        item->when = t + TIMEOUT;

                        /* Set up a timeout that will be called every 0.5
                         * seconds */
                        item->timeout_id =
                                g_timeout_add (500,
                                               (GSourceFunc) timeout, 
                                               applet);

                        /* Add a new launch at the end of list */
                        applet->launch_list =
                                g_list_append (applet->launch_list,
                                               (gpointer) item);

                        if (!applet->hourglass_shown)
                                show_hourglass (applet);
                }
                break;

        case SN_MONITOR_EVENT_COMPLETED:
        case SN_MONITOR_EVENT_CANCELED:
                {
                        GList *tmp = applet->launch_list;

                        /* Find actual list item and free it*/
                        while( tmp != NULL) {
                                LaunchItem *item = (LaunchItem*) tmp->data;
                                if (!strcmp (item->id, id)) {
                                        applet->launch_list =
                                                g_list_remove (tmp, item);
                                        g_source_remove (item->timeout_id);
                                        free (item->id);
                                        free (item);

                                        break;
                                }
                                tmp = tmp->next;
                        }

                        if (applet->launch_list == NULL &&
                            applet->hourglass_shown)
                                hide_hourglass (applet);
                }
                break;
        default:
                break;                /* Nothing */
        }
}

static gboolean
timeout (StartupApplet *applet)
{
        time_t t;
        char *icon;

        if (!applet->hourglass_shown)
                return TRUE;

        t = time (NULL);
        GList *tmp = applet->launch_list;

        /* handle launchee timeouts */
        while (tmp != NULL) {
                GList *tmp_next = tmp->next;
                LaunchItem *item = (LaunchItem *) tmp->data;
                if ((item->when - t) <= 0) {
                        applet->launch_list = g_list_delete_link (applet->launch_list, tmp);
                        g_source_remove (item->timeout_id);
                        free (item->id);
                        free (item);

                        break;
                }
                tmp = tmp_next;
        }

        if (applet->launch_list == NULL && applet->hourglass_shown) {
                hide_hourglass (applet);
                return TRUE;
        }

        applet->hourglass_cur_frame_n++;
        if (applet->hourglass_cur_frame_n == 8)
                applet->hourglass_cur_frame_n = 0;

        icon = malloc (sizeof(DATADIR) + 16);
        sprintf (icon, "%s/hourglass-%i.png", DATADIR,
                 applet->hourglass_cur_frame_n);

        mb_panel_scaling_image_set_icon (applet->image, icon);

        free (icon);

        return TRUE;
}

static GdkFilterReturn
filter_func (GdkXEvent     *gdk_xevent,
             GdkEvent      *event,
             StartupApplet *applet)
{
        XEvent *xevent;
        xevent = (XEvent *) gdk_xevent;

        sn_display_process_event (applet->sn_display, xevent);

        return GDK_FILTER_CONTINUE;
}

G_MODULE_EXPORT GtkWidget *
mb_panel_applet_create (const char    *id,
                        GtkOrientation orientation)
{
        StartupApplet *applet;
        Display *xdisplay;

        /* Create applet data structure */
        applet = g_slice_new0 (StartupApplet);

        applet->launch_list = NULL;
        applet->hourglass_shown = FALSE;

        /* Create image */
        applet->image = MB_PANEL_SCALING_IMAGE
                              (mb_panel_scaling_image_new (orientation, NULL));
        mb_panel_scaling_image_set_caching (applet->image, TRUE);

        gtk_widget_set_name (GTK_WIDGET(applet->image),
                             "MatchboxPanelStartupMonitor");

        g_object_weak_ref (G_OBJECT(applet->image),
                           (GWeakNotify) startup_applet_free, applet);

        xdisplay = GDK_DISPLAY_XDISPLAY
                                (gtk_widget_get_display
                                        (GTK_WIDGET (applet->image)));

        applet->sn_display = sn_display_new (xdisplay, NULL, NULL);

        applet->sn_context = sn_monitor_context_new (applet->sn_display,
                                                     DefaultScreen(xdisplay),
                                                     monitor_event_func,
                                                     (void *) applet,
                                                     NULL);

        /* We have to select for property events on at least one
         * root window (but not all as INITIATE messages go to
         * all root windows)
         */
        XSelectInput (xdisplay,
                      DefaultRootWindow(xdisplay), PropertyChangeMask);

        applet->root_window =
                gdk_window_lookup_for_display
                        (gdk_x11_lookup_xdisplay (xdisplay), 0);

        gdk_window_add_filter (applet->root_window,
                               (GdkFilterFunc) filter_func, applet);

        return GTK_WIDGET (applet->image);
}
