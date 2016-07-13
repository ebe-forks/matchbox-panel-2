/*
 * (C) 2006 OpenedHand Ltd.
 *
 * Author: Jorn Baayen <jorn@openedhand.com>
 *
 * Licensed under the GPL v2 or greater.
 */

#include "battery.h"
#include <matchbox-panel/mb-panel.h>
#include <matchbox-panel/mb-panel-scaling-image2.h>

typedef struct {
        MBPanelScalingImage2 *image;
        const char *last_icon;
        guint timeout_id;
} BatteryApplet;

static void
battery_applet_free (BatteryApplet *applet)
{
        g_source_remove (applet->timeout_id);

        g_slice_free (BatteryApplet, applet);
}

static gboolean
timeout (BatteryApplet *applet)
{
	const char *icon;

	icon = pm_battery_icon ();

	if (!icon)
		return FALSE;

        /* Don't recreate pixbuf if we will display the same image */
        if (icon == applet->last_icon)
                return TRUE;

        applet->last_icon = icon;

        mb_panel_scaling_image2_set_icon (applet->image, icon);

        /* Keep going */
        return TRUE;
}

G_MODULE_EXPORT GtkWidget *
mb_panel_applet_create (const char    *id,
                        GtkOrientation orientation)
{
        BatteryApplet *applet;
        GtkWidget *image;

        /* Check that we have PM support */
        if (!pm_support())
                return NULL;

        applet = g_slice_new (BatteryApplet);

        applet->last_icon = NULL;

        image = mb_panel_scaling_image2_new (orientation, NULL);
        applet->image = MB_PANEL_SCALING_IMAGE2 (image);

        gtk_widget_set_name (image, "MatchboxPanelBatteryMonitor");

        g_object_weak_ref (G_OBJECT (image),
                           (GWeakNotify) battery_applet_free,
                           applet);

        /* Set up a timeout that will be called every 5 minutes */
        applet->timeout_id = g_timeout_add_seconds (5 * 60,
                                                    (GSourceFunc) timeout,
                                                    applet);

        timeout (applet);

        gtk_widget_show (image);

        return image;
}
