/* 
 * (C) 2006 OpenedHand Ltd.
 *
 * Author: Jorn Baayen <jorn@openedhand.com>
 *
 * Licensed under the GPL v2 or greater.
 */

#ifndef __MATCHBOX_PANEL_H__
#define __MATCHBOX_PANEL_H__

#include <gtk/gtkwidget.h>

G_BEGIN_DECLS

G_MODULE_IMPORT GtkWidget *
mb_panel_applet_create (const char *id,
                        int         panel_width,
                        int         panel_height);

G_END_DECLS

#endif /* __MATCHBOX_PANEL_H__ */
