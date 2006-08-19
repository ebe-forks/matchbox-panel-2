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

/* Applet destroyed */
static void
destroy_cb (GtkLabel *label)
{
        guint timeout_id;
        
        /* Remove timeout */
        timeout_id =
                GPOINTER_TO_UINT
                        (g_object_get_data (G_OBJECT (label), "timeout-id"));

        g_source_remove (timeout_id);
}

/* Called every minute */
static gboolean
timeout (GtkLabel *label)
{
        time_t t;
        char str[6];

        /* Update label */
        t = time (NULL);
        strftime (str, 6, "%H:%M", localtime (&t));
        
        gtk_label_set_text (label, str);

        /* Keep going */
        return TRUE;
}

/* Called on the next minute after applet creation */
static gboolean
initial_timeout (GtkLabel *label)
{
        guint timeout_id;
        
        /* Update label */
        timeout (label);
        
        /* Install a new timeout that is called every minute */
        timeout_id = g_timeout_add (60 * 1000,
                                    (GSourceFunc) timeout,
                                    label);
        
        g_object_set_data (G_OBJECT (label),
                           "timeout-id",
                           GUINT_TO_POINTER (timeout_id));
        
        /* Don't call this again */
        return FALSE;
}

/* GtkStyle set */
static void
style_set_cb (GtkWidget *widget,
              GtkStyle  *style,
              gpointer   user_data)
{
        GtkRequisition requisition;

        gtk_widget_size_request (widget, &requisition);

        /* Do we have enough space for a horizontal clock? */
        if (GPOINTER_TO_INT (user_data) < requisition.width) {
                /* No: Rotate label 90 degrees */
                gtk_label_set_angle (GTK_LABEL (widget), 90.0);
        }
}

G_MODULE_EXPORT GtkWidget *
mb_panel_applet_create (const char *id,
                        int         panel_width,
                        int         panel_height)
{
        GtkWidget *label;
        guint timeout_id;
        time_t t;
        struct tm *local_time;

        /* Create label */
        label = gtk_label_new (NULL);

        gtk_widget_set_name (label, "MatchboxPanelClock");

        timeout (GTK_LABEL (label));

        g_signal_connect (label,
                          "destroy",
                          G_CALLBACK (destroy_cb),
                          NULL);

        /* Set up a timeout to be called when we hit the next minute */
        t = time (NULL);
        local_time = localtime (&t);
        
        timeout_id = g_timeout_add ((60 - local_time->tm_sec) * 1000,
                                    (GSourceFunc) initial_timeout,
                                    label);
        
        g_object_set_data (G_OBJECT (label),
                           "timeout-id",
                           GUINT_TO_POINTER (timeout_id));

        /* Is this a vertical panel? */
        if (MIN (panel_width, panel_height) == panel_width) {
                /* Yes: Connect to 'style-set' signal */
                g_signal_connect (label,
                                  "style-set",
                                  G_CALLBACK (style_set_cb),
                                  GINT_TO_POINTER (panel_width));

                gtk_misc_set_padding (GTK_MISC (label), 0, 5);
        } else
                gtk_misc_set_padding (GTK_MISC (label), 5, 0);

        /* Show! */
        gtk_widget_show (label);

        return label;
};
