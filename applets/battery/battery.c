/* 
 * (C) 2006 OpenedHand Ltd.
 *
 * Author: Jorn Baayen <jorn@openedhand.com>
 *
 * Licensed under the GPL v2 or greater.
 */

#include <apm.h>
#include <matchbox-panel/mb-panel.h>
#include <matchbox-panel/mb-panel-scaling-image.h>

typedef struct {
        MBPanelScalingImage *image;
        const char *last_icon;

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
        const char *icon;

        apm_read (&info);

        if (info.battery_status == BATTERY_STATUS_ABSENT)
                icon = DATADIR "/ac-adapter.png";
        else {
                if (info.ac_line_status == AC_LINE_STATUS_ON) {
                        if (info.battery_percentage < 10)
                                icon = DATADIR "battery-charging-000.png";
                        else if (info.battery_percentage < 30)
                                icon = DATADIR "battery-charging-020.png";
                        else if (info.battery_percentage < 50)
                                icon = DATADIR "battery-charging-040.png";
                        else if (info.battery_percentage < 70)
                                icon = DATADIR "battery-charging-060.png";
                        else if (info.battery_percentage < 90)
                                icon = DATADIR "battery-charging-080.png";
                        else
                                icon = DATADIR "battery-charging-100.png";
                } else {
                        if (info.battery_percentage < 10)
                                icon = DATADIR "battery-discharging-000.png";
                        else if (info.battery_percentage < 30)
                                icon = DATADIR "battery-discharging-020.png";
                        else if (info.battery_percentage < 50)
                                icon = DATADIR "battery-discharging-040.png";
                        else if (info.battery_percentage < 70)
                                icon = DATADIR "battery-discharging-060.png";
                        else if (info.battery_percentage < 90)
                                icon = DATADIR "battery-discharging-080.png";
                        else
                                icon = DATADIR "battery-discharging-100.png";
                }
        }

        /* Don't recreate pixbuf if we will display the same image */
        if (icon == applet->last_icon)
                return TRUE;

        applet->last_icon = icon;
        
        /* Load icon */
        mb_panel_scaling_image_set_icon (applet->image, icon);

        /* Keep going */
        return TRUE;
}

G_MODULE_EXPORT GtkWidget *
mb_panel_applet_create (const char    *id,
                        GtkOrientation orientation)
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

        applet->last_icon = NULL;

        /* Create image */
        image = mb_panel_scaling_image_new (NULL);
        applet->image = MB_PANEL_SCALING_IMAGE (image);

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
