/* 
 * (C) 2006 OpenedHand Ltd.
 *
 * Author: Jorn Baayen <jorn@openedhand.com>
 *
 * Licensed under the GPL v2 or greater.
 */

#include <gtk/gtkimage.h>
#include <apm.h>
#include <matchbox-panel/mb-panel.h>

/* Applet destroyed */
static void
destroy_cb (GtkImage *image)
{
        guint timeout_id;
        
        /* Remove timeout */
        timeout_id =
                GPOINTER_TO_UINT
                        (g_object_get_data (G_OBJECT (image), "timeout-id"));

        g_source_remove (timeout_id);
}

/* Called every minute */
static gboolean
timeout (GtkImage *image)
{
        apm_info info;

        apm_read (&info);

        /* TODO choose appropriate graphics to display */

        /* Keep going */
        return TRUE;
}

G_MODULE_EXPORT GtkWidget *
mb_panel_applet_create (const char *id,
                        int         panel_width,
                        int         panel_height)
{
        GtkWidget *image;
        guint timeout_id;

        /* Create label */
        image = gtk_image_new ();

        gtk_widget_set_name (image, "MatchboxPanelBatteryMonitor");

        timeout (GTK_IMAGE (image));

        g_signal_connect (image,
                          "destroy",
                          G_CALLBACK (destroy_cb),
                          NULL);

        /* Set up a timeout that will be called every minute */
        timeout_id = g_timeout_add (60 * 1000,
                                    (GSourceFunc) timeout,
                                    image);
        
        g_object_set_data (G_OBJECT (image),
                           "timeout-id",
                           GUINT_TO_POINTER (timeout_id));

        /* Show! */
        gtk_widget_show (image);

        return image;
};
