/*
 * (C) 2013 Intel Corp
 *
 * Author: Ross Burton <ross.burton@intel.com>
 *
 * Licensed under the GPL v2 or greater.
 */

#ifndef __MB_PANEL_SCALING_IMAGE2_H__
#define __MB_PANEL_SCALING_IMAGE2_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define MB_PANEL_TYPE_SCALING_IMAGE2 \
                (mb_panel_scaling_image2_get_type ())
#define MB_PANEL_SCALING_IMAGE2(obj) \
                (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                 MB_PANEL_TYPE_SCALING_IMAGE2, \
                 MBPanelScalingImage2))
#define MB_PANEL_SCALING_IMAGE2_CLASS(klass) \
                (G_TYPE_CHECK_CLASS_CAST ((klass), \
                 MB_PANEL_TYPE_SCALING_IMAGE2, \
                 MBPanelScalingImage2Class))
#define MB_PANEL_IS_SCALING_IMAGE2(obj) \
                (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                 MB_PANEL_TYPE_SCALING_IMAGE2))
#define MB_PANEL_IS_SCALING_IMAGE2_CLASS(klass) \
                (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                 MB_PANEL_TYPE_SCALING_IMAGE2))
#define MB_PANEL_SCALING_IMAGE2_GET_CLASS(obj) \
                (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                 MB_PANEL_TYPE_SCALING_IMAGE2, \
                 MBPanelScalingImage2Class))

typedef struct _MBPanelScalingImage2Private MBPanelScalingImage2Private;

typedef struct {
        GtkDrawingArea parent;
        MBPanelScalingImage2Private *priv;
} MBPanelScalingImage2;

typedef struct {
        GtkDrawingAreaClass parent_class;
} MBPanelScalingImage2Class;

GType
mb_panel_scaling_image2_get_type (void) G_GNUC_CONST;

GtkWidget *
mb_panel_scaling_image2_new         (GtkOrientation       orientation,
                                    const char          *icon);

void
mb_panel_scaling_image2_set_icon    (MBPanelScalingImage2 *image,
                                    const char          *icon);

const char *
mb_panel_scaling_image2_get_icon    (MBPanelScalingImage2 *image);

G_END_DECLS

#endif /* __MB_PANEL_SCALING_IMAGE_H__ */
