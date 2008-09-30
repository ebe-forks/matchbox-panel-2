/*
 * (C) 2008 Intel.
 *
 * Licensed under the GPL v2 or greater.
 */

#include "battery.h"
#include <apm.h>

int pm_support(void)
{
	if (1 == apm_exists ()) {
                g_warning ("No APM support");

                return 0;
        }

	return 1;
}

const char* pm_battery_icon(void)
{
	apm_info info;
        const char *icon;

        memset (&info, 0, sizeof (apm_info));
        apm_read (&info);

        if (info.battery_status == BATTERY_STATUS_ABSENT)
                icon = "ac-adapter.png";
        else {
                if (info.ac_line_status == AC_LINE_STATUS_ON) {
                        if (info.battery_percentage < 10)
                                icon = "battery-charging-000.png";
                        else if (info.battery_percentage < 30)
                                icon = "battery-charging-020.png";
                        else if (info.battery_percentage < 50)
                                icon = "battery-charging-040.png";
                        else if (info.battery_percentage < 70)
                                icon = "battery-charging-060.png";
                        else if (info.battery_percentage < 90)
                                icon = "battery-charging-080.png";
                        else
                                icon = "battery-charging-100.png";
                } else {
                        if (info.battery_percentage < 10)
                                icon = "battery-discharging-000.png";
                        else if (info.battery_percentage < 30)
                                icon = "battery-discharging-020.png";
                        else if (info.battery_percentage < 50)
                                icon = "battery-discharging-040.png";
                        else if (info.battery_percentage < 70)
                                icon = "battery-discharging-060.png";
                        else if (info.battery_percentage < 90)
                                icon = "battery-discharging-080.png";
                        else
                                icon = "battery-discharging-100.png";
                }
        }

	return icon;
}
