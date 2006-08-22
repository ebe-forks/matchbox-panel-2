/* 
 * (C) 2006 OpenedHand Ltd.
 *
 * Author: Jorn Baayen <jorn@openedhand.com>
 *
 * Licensed under the GPL v2 or greater.
 */

#include <gtk/gtklabel.h>
#include <time.h>
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
        char str[6];

        /* Update label */
        t = time (NULL);
        strftime (str, 6, "%H:%M", localtime (&t));
        
        gtk_label_set_text (applet->label, str);

        /* Keep going */
        return TRUE;
}

/* Called on the next minute after applet creation */
static gboolean
initial_timeout (ClockApplet *applet)
{
        /* Update label */
        timeout (applet);
        
        /* Install a new timeout that is called every minute */
        applet->timeout_id = g_timeout_add (60 * 1000,
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

        /* Create applet data structure */
        applet = g_slice_new (ClockApplet);

        /* Create label */
        label = gtk_label_new (NULL);
        applet->label = GTK_LABEL (label);

        gtk_widget_set_name (label, "MatchboxPanelClock");

        g_object_weak_ref (G_OBJECT (label),
                           (GWeakNotify) clock_applet_free,
                           applet);

        /* Is this a vertical panel? */
        if (orientation == GTK_ORIENTATION_VERTICAL) {
                /* Yes: Rotate label */
                gtk_label_set_angle (GTK_LABEL (label), 90.0);
        }
        
        /* Set up a timeout to be called when we hit the next minute */
        t = time (NULL);
        local_time = localtime (&t);
        
        applet->timeout_id = g_timeout_add ((60 - local_time->tm_sec) * 1000,
                                            (GSourceFunc) initial_timeout,
                                            applet);
        
        timeout (applet);

        /* Show! */
        gtk_widget_show (label);

        return label;
};
