/*
 * (C) 2008 Intel.
 *
 * Licensed under the GPL v2 or greater.
 */

#include "battery.h"
#include <libacpi.h>

global_t global;
adapter_t *ac;
int batt_state, ac_state;

int pm_support(void)
{
	if(check_acpi_support() == NOT_SUPPORTED){
		g_warning("No ACPI support\n");

		return 0;
	}

	ac = &global.adapt;

	batt_state = init_acpi_batt(&global);
	ac_state = init_acpi_acadapt(&global);

	return 1;
}

const char* pm_battery_icon(void)
{
	const char *icon;
	battery_t *binfo;

	if (batt_state != SUCCESS) {
		g_warning("Couldnt initialize ACPI battery\n");
		return NULL;
	}

	read_acpi_batt(0);
	read_acpi_acstate(&global);

	binfo = &batteries[0];

	if (binfo->batt_state == B_ERR) {
		g_warning("Couldn't read battery state\n");
		return NULL;
	}

	if (!binfo->present) {
		/* Battery is removed */
		icon = "ac-adapter.png";
		return icon;
	}

	if(ac_state == SUCCESS && ac->ac_state == P_AC) {
		/* We're charging */
		if (binfo->percentage < 10)
			icon = "battery-charging-000.png";
		else if (binfo->percentage < 30)
			icon = "battery-charging-020.png";
		else if (binfo->percentage < 50)
			icon = "battery-charging-040.png";
		else if (binfo->percentage < 70)
			icon = "battery-charging-060.png";
		else if (binfo->percentage < 90)
			icon = "battery-charging-080.png";
		else
			icon = "battery-charging-100.png";

	} else {
		if (binfo->percentage < 10)
			icon = "battery-discharging-000.png";
		else if (binfo->percentage < 30)
			icon = "battery-discharging-020.png";
		else if (binfo->percentage < 50)
			icon = "battery-discharging-040.png";
		else if (binfo->percentage < 70)
			icon = "battery-discharging-060.png";
		else if (binfo->percentage < 90)
			icon = "battery-discharging-080.png";
		else
			icon = "battery-discharging-100.png";
	}

	return icon;
}
