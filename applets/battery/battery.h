/*
 * (C) 2008 Intel.
 *
 * Licensed under the GPL v2 or greater.
 */

#ifndef MB_APPLET_BATTERY_H
#define MB_APPLET_BATTERY_H

#include <string.h>
#include <matchbox-panel/mb-panel.h>
#include <matchbox-panel/mb-panel-scaling-image.h>

int pm_support(void);
const char* pm_battery_icon(void);

#endif
