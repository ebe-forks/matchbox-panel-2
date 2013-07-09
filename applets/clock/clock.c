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
#include <time.h>
#include <gtk/gtk.h>
#include <matchbox-panel/mb-panel.h>

typedef struct {
        GtkLabel *label;

        guint timeout_id;
} ClockApplet;

static void
clock_applet_free (ClockApplet *applet)
{
        g_source_remove (applet->timeout_id);

        g_slice_free (ClockApplet, applet);
}

/* Called every minute */
static gboolean
timeout (ClockApplet *applet)
{
        time_t t;
        char str[6], *markup;

        t = time (NULL);
        strftime (str, sizeof (str), "%H:%M", localtime (&t));

        markup = g_strdup_printf("<b>%s</b>", str);

        gtk_label_set_markup (applet->label, markup);

        g_free (markup);

        /* Keep going */
        return TRUE;
}

/* Called on the next minute after applet creation */
static gboolean
initial_timeout (ClockApplet *applet)
{
        timeout (applet);

        /* Install a new timeout that is called every minute */
        applet->timeout_id = g_timeout_add_seconds (60,
                                                    (GSourceFunc) timeout,
                                                    applet);

        /* Don't call this again */
        return FALSE;
}

G_MODULE_EXPORT GtkWidget *
mb_panel_applet_create (const char    *id,
                        GtkOrientation orientation)
{
        ClockApplet *applet;
        GtkWidget *label;
        time_t t;
        struct tm *local_time;

        applet = g_slice_new0 (ClockApplet);

        label = gtk_label_new (NULL);
        applet->label = GTK_LABEL (label);

        gtk_widget_set_name (label, "MatchboxPanelClock");

        g_object_weak_ref (G_OBJECT (label),
                           (GWeakNotify) clock_applet_free,
                           applet);

        if (orientation == GTK_ORIENTATION_VERTICAL) {
                gtk_label_set_angle (GTK_LABEL (label), 90.0);
        }

        /* Set up a timeout to be called when we hit the next minute */
        t = time (NULL);
        local_time = localtime (&t);

        applet->timeout_id = g_timeout_add_seconds (60 - local_time->tm_sec,
                                                    (GSourceFunc) initial_timeout,
                                                    applet);

        timeout (applet);

        gtk_widget_show (label);

        return label;
};
