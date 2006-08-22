/* 
 * (C) 2006 OpenedHand Ltd.
 *
 * Author: Jorn Baayen <jorn@openedhand.com>
 *
 * Licensed under the GPL v2 or greater.
 */

#ifndef __MB_PANEL_SCALING_IMAGE_H__
#define __MB_PANEL_SCALING_IMAGE_H__

#include <gtk/gtkimage.h>

G_BEGIN_DECLS

#define MB_PANEL_TYPE_SCALING_IMAGE \
                (mb_panel_scaling_image_get_type ())
#define MB_PANEL_SCALING_IMAGE(obj) \
                (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                 MB_PANEL_TYPE_SCALING_IMAGE, \
                 MBPanelScalingImage))
#define MB_PANEL_SCALING_IMAGE_CLASS(klass) \
                (G_TYPE_CHECK_CLASS_CAST ((klass), \
                 MB_PANEL_TYPE_SCALING_IMAGE, \
                 MBPanelScalingImageClass))
#define MB_PANEL_IS_SCALING_IMAGE(obj) \
                (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                 MB_PANEL_TYPE_SCALING_IMAGE))
#define MB_PANEL_IS_SCALING_IMAGE_CLASS(klass) \
                (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                 MB_PANEL_TYPE_SCALING_IMAGE))
#define MB_PANEL_SCALING_IMAGE_GET_CLASS(obj) \
                (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                 MB_PANEL_TYPE_SCALING_IMAGE, \
                 MBPanelScalingImageClass))

typedef struct _MBPanelScalingImagePrivate MBPanelScalingImagePrivate;

typedef struct {
        GtkImage parent;

        MBPanelScalingImagePrivate *priv;
} MBPanelScalingImage;

typedef struct {
        GtkImageClass parent_class;

        /* Future padding */
        gpointer (* _reserved1) (void);
        gpointer (* _reserved2) (void);
        gpointer (* _reserved3) (void);
        gpointer (* _reserved4) (void);
} MBPanelScalingImageClass;

GType
mb_panel_scaling_image_get_type (void) G_GNUC_CONST;

GtkWidget *
mb_panel_scaling_image_new      (const char          *icon);

void
mb_panel_scaling_image_set_icon (MBPanelScalingImage *image,
                                 const char          *icon);

const char *
mb_panel_scaling_image_get_icon (MBPanelScalingImage *image);

G_END_DECLS

#endif /* __MB_PANEL_SCALING_IMAGE_H__ */
