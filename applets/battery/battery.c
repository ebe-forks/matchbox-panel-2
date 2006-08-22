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

typedef struct {
        GtkImage *image;
        guint size;
        const char *last_img;

        guint timeout_id;
} BatteryApplet;

/* Applet destroyed */
static void
battery_applet_free (BatteryApplet *applet)
{
        g_source_remove (applet->timeout_id);

        g_slice_free (BatteryApplet, applet);
}

/* Called every 5 minutes */
static gboolean
timeout (BatteryApplet *applet)
{
        apm_info info;
        const char *img;
        GdkPixbuf *pixbuf;
        GError *error;

        apm_read (&info);

        if (info.battery_status == BATTERY_STATUS_ABSENT)
                img = DATADIR "/ac-adapter.png";
        else {
                if (info.ac_line_status == AC_LINE_STATUS_ON) {
                        if (info.battery_percentage < 10)
                                img = DATADIR "battery-charging-000.png";
                        else if (info.battery_percentage < 30)
                                img = DATADIR "battery-charging-020.png";
                        else if (info.battery_percentage < 50)
                                img = DATADIR "battery-charging-040.png";
                        else if (info.battery_percentage < 70)
                                img = DATADIR "battery-charging-060.png";
                        else if (info.battery_percentage < 90)
                                img = DATADIR "battery-charging-080.png";
                        else
                                img = DATADIR "battery-charging-100.png";
                } else {
                        if (info.battery_percentage < 10)
                                img = DATADIR "battery-discharging-000.png";
                        else if (info.battery_percentage < 30)
                                img = DATADIR "battery-discharging-020.png";
                        else if (info.battery_percentage < 50)
                                img = DATADIR "battery-discharging-040.png";
                        else if (info.battery_percentage < 70)
                                img = DATADIR "battery-discharging-060.png";
                        else if (info.battery_percentage < 90)
                                img = DATADIR "battery-discharging-080.png";
                        else
                                img = DATADIR "battery-discharging-100.png";
                }
        }

        /* Don't recreate pixbuf if we will display the same image */
        if (img == applet->last_img)
                return TRUE;

        applet->last_img = img;

        /* Load pixbuf */
        error = NULL;
        pixbuf = gdk_pixbuf_new_from_file_at_scale (img,
                                                    applet->size,
                                                    applet->size,
                                                    TRUE,
                                                    &error);
        if (pixbuf) {
                gtk_image_set_from_pixbuf (applet->image, pixbuf);
        
                g_object_unref (pixbuf);
        } else {
                g_warning (error->message);

                g_error_free (error);
        }

        /* Keep going */
        return TRUE;
}

G_MODULE_EXPORT GtkWidget *
mb_panel_applet_create (const char *id,
                        int         panel_width,
                        int         panel_height)
{
        BatteryApplet *applet;
        GtkWidget *image;

        /* Check that we have APM support */
        if (!apm_exists ()) {
                g_warning ("No APM support");

                return NULL;
        }

        /* Create applet data structure */
        applet = g_slice_new (BatteryApplet);

        applet->size = MIN (panel_width, panel_height);
        applet->last_img = NULL;

        /* Create image */
        image = gtk_image_new ();
        applet->image = GTK_IMAGE (image);

        gtk_widget_set_name (image, "MatchboxPanelBatteryMonitor");

        g_object_weak_ref (G_OBJECT (image),
                           (GWeakNotify) battery_applet_free,
                           applet);

        /* Set up a timeout that will be called every 5 minutes */
        applet->timeout_id = g_timeout_add (5 * 60 * 1000,
                                            (GSourceFunc) timeout,
                                            applet);

        timeout (applet);

        /* Show! */
        gtk_widget_show (image);

        return image;
}
