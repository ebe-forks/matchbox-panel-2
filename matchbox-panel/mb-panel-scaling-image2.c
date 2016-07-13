/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/*
 * (C) 2013 Intel Corp
 *
 * Author: Ross Burton <ross.burton@intel.com>
 *
 * Licensed under the GPL v2 or greater.
 */

#include <config.h>
#include <string.h>
#include <gtk/gtk.h>

#include "mb-panel-scaling-image2.h"

struct _MBPanelScalingImage2Private {
        GtkOrientation orientation;
        char *icon;
        int size;
        GtkIconTheme *icon_theme;
        guint icon_theme_changed_id;
        GdkPixbuf *pixbuf;
};

enum {
        PROP_0,
        PROP_ORIENTATION,
        PROP_ICON,
};

G_DEFINE_TYPE (MBPanelScalingImage2,
               mb_panel_scaling_image2,
               GTK_TYPE_DRAWING_AREA);

static void
mb_panel_scaling_image2_init (MBPanelScalingImage2 *image)
{
        image->priv = G_TYPE_INSTANCE_GET_PRIVATE (image,
                                                   MB_PANEL_TYPE_SCALING_IMAGE2,
                                                   MBPanelScalingImage2Private);

        image->priv->orientation = GTK_ORIENTATION_HORIZONTAL;

        image->priv->icon = NULL;

        image->priv->pixbuf = NULL;
}

static void
mb_panel_scaling_image2_set_property (GObject      *object,
                                     guint         property_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
        MBPanelScalingImage2 *image;

        image = MB_PANEL_SCALING_IMAGE2 (object);

        switch (property_id) {
        case PROP_ORIENTATION:
                image->priv->orientation = g_value_get_enum (value);
                break;
        case PROP_ICON:
                mb_panel_scaling_image2_set_icon (image,
                                                 g_value_get_string (value));
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
                break;
        }
}

static void
mb_panel_scaling_image2_get_property (GObject    *object,
                                     guint       property_id,
                                     GValue     *value,
                                     GParamSpec *pspec)
{
        MBPanelScalingImage2 *image;

        image = MB_PANEL_SCALING_IMAGE2 (object);

        switch (property_id) {
        case PROP_ORIENTATION:
                g_value_set_enum (value, image->priv->orientation);
                break;
        case PROP_ICON:
                g_value_set_string (value, image->priv->icon);
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
                break;
        }
}

static void
mb_panel_scaling_image2_dispose (GObject *object)
{
        MBPanelScalingImage2 *image;
        GObjectClass *object_class;

        object_class = G_OBJECT_CLASS (mb_panel_scaling_image2_parent_class);
        image = MB_PANEL_SCALING_IMAGE2 (object);

        if (image->priv->icon_theme_changed_id) {
                g_signal_handler_disconnect
                        (image->priv->icon_theme,
                         image->priv->icon_theme_changed_id);
                image->priv->icon_theme_changed_id = 0;
        }

        object_class->dispose (object);
}

static void
mb_panel_scaling_image2_finalize (GObject *object)
{
        MBPanelScalingImage2 *image;
        GObjectClass *object_class;

        object_class = G_OBJECT_CLASS (mb_panel_scaling_image2_parent_class);
        image = MB_PANEL_SCALING_IMAGE2 (object);

        g_free (image->priv->icon);

        object_class->finalize (object);
}

/* Strips extension off filename */
static char *
strip_extension (const char *file)
{
        char *stripped, *p;

        stripped = g_strdup (file);

        p = strrchr (stripped, '.');
        if (p &&
            (!strcmp (p, ".png") ||
             !strcmp (p, ".svg") ||
             !strcmp (p, ".xpm")))
	        *p = 0;

        return stripped;
}

/* Find icon filename */
/* This follows the same logic as gnome-panel. This should hopefully
 * ensure correct behaviour. */
static char *
find_icon (GtkIconTheme *icon_theme,
           const char   *icon,
           int           size)
{
        GtkIconInfo *info;
        char *new_icon, *stripped, *prefixed;

        if (g_path_is_absolute (icon)) {
                if (g_file_test (icon, G_FILE_TEST_EXISTS))
                        return g_strdup (icon);
                else
                        new_icon = g_path_get_basename (icon);
        } else
                new_icon = (char *) icon;

        stripped = strip_extension (new_icon);

        if (new_icon != icon)
                g_free (new_icon);

        prefixed = g_strconcat ("panel-", stripped, NULL);

        info = gtk_icon_theme_lookup_icon (icon_theme,
                                           prefixed,
                                           size,
                                           0);

        if (info == NULL) {
          info = gtk_icon_theme_lookup_icon (icon_theme,
                                             stripped,
                                             size,
                                             0);
        }

        g_free (stripped);
        g_free (prefixed);

        if (info) {
                char *file;

                file = g_strdup (gtk_icon_info_get_filename (info));

                gtk_icon_info_free (info);

                return file;
        } else
                return NULL;
}

/* Reload the specified icon */
static void
reload_icon (MBPanelScalingImage2 *image, gboolean force)
{
        GtkAllocation alloc;
        int size;
        char *file;
        GError *error;
        GdkPixbuf *pixbuf;

        /* Determine the required icon size */
        gtk_widget_get_allocation (GTK_WIDGET (image), &alloc);
        size = image->priv->orientation == GTK_ORIENTATION_HORIZONTAL
                ? alloc.height : alloc.width;

        if (!force && size == image->priv->size) {
                return;
        }

        image->priv->size = size;

        if (!image->priv->icon) {
                g_clear_object (&image->priv->pixbuf);
                gtk_widget_queue_resize (GTK_WIDGET (image));
                return;
        }

        file = find_icon (image->priv->icon_theme,
                          image->priv->icon,
                          size);
	if (!file) {
                g_warning ("Icon \"%s\" not found", image->priv->icon);

                return;
        }

        error = NULL;
        pixbuf = gdk_pixbuf_new_from_file_at_scale (file, size, size, TRUE, &error);
        g_free (file);

        g_clear_object (&image->priv->pixbuf);
        if (pixbuf) {
                image->priv->pixbuf = pixbuf;
        } else {
                g_warning ("No pixbuf found: %s", error->message);
                g_error_free (error);
        }

        gtk_widget_queue_resize (GTK_WIDGET (image));
}

/* Icon theme changed */
static void
icon_theme_changed_cb (GtkIconTheme   *icon_theme,
                       MBPanelScalingImage2 *image)
{
        GtkWidget *widget = GTK_WIDGET (image);

        if (gtk_widget_get_realized (widget)) {
                reload_icon (image, TRUE);
        }
}

static void
mb_panel_scaling_image2_size_allocate (GtkWidget *widget,
                                      GtkAllocation *allocation)
{
        MBPanelScalingImage2 *image = MB_PANEL_SCALING_IMAGE2 (widget);
        GtkWidgetClass *widget_class;

        widget_class = GTK_WIDGET_CLASS (mb_panel_scaling_image2_parent_class);
        widget_class->size_allocate (widget, allocation);

        if (gtk_widget_get_realized (widget)) {
                reload_icon (image, FALSE);
        }
}

static void
mb_panel_scaling_image2_realize (GtkWidget *widget)
{
        GtkWidgetClass *widget_class;

        reload_icon (MB_PANEL_SCALING_IMAGE2 (widget), TRUE);

        widget_class = GTK_WIDGET_CLASS (mb_panel_scaling_image2_parent_class);
        widget_class->realize (widget);
}

static void
mb_panel_scaling_image2_screen_changed (GtkWidget *widget,
                                        GdkScreen *old_screen)
{
        MBPanelScalingImage2 *image;
        GdkScreen *screen;
        GtkIconTheme *new_icon_theme;

        if (GTK_WIDGET_CLASS (mb_panel_scaling_image2_parent_class)->screen_changed)
                GTK_WIDGET_CLASS (mb_panel_scaling_image2_parent_class)->screen_changed (widget, old_screen);

        image = MB_PANEL_SCALING_IMAGE2 (widget);
        screen = gtk_widget_get_screen (widget);
        new_icon_theme = gtk_icon_theme_get_for_screen (screen);
        if (image->priv->icon_theme == new_icon_theme)
                return;

        if (image->priv->icon_theme_changed_id) {
                g_signal_handler_disconnect
                        (image->priv->icon_theme,
                         image->priv->icon_theme_changed_id);
        }

        image->priv->icon_theme = new_icon_theme;

        image->priv->icon_theme_changed_id =
                g_signal_connect (image->priv->icon_theme,
                                  "changed",
                                  G_CALLBACK (icon_theme_changed_cb),
                                  image);

        if (gtk_widget_get_realized (widget)) {
                reload_icon (MB_PANEL_SCALING_IMAGE2 (widget), TRUE);
        }
}
static gboolean
mb_panel_scaling_image2_draw (GtkWidget *widget, cairo_t *cr)
{
        MBPanelScalingImage2 *image = MB_PANEL_SCALING_IMAGE2 (widget);

        if (image->priv->pixbuf) {
                gdk_cairo_set_source_pixbuf (cr, image->priv->pixbuf, 0.0, 0.0);
                cairo_paint (cr);
        }
        return TRUE;
}

static void
mb_panel_scaling_image2_get_preferred_width (GtkWidget *widget, int *minimum_width, int *natural_width)
{
        MBPanelScalingImage2 *image = MB_PANEL_SCALING_IMAGE2 (widget);

        if (image->priv->pixbuf) {
                *minimum_width = *natural_width = gdk_pixbuf_get_width (image->priv->pixbuf);
        } else {
                *minimum_width = *natural_width = 1;
        }
}

static void
mb_panel_scaling_image2_class_init (MBPanelScalingImage2Class *klass)
{
        GObjectClass *object_class;
        GtkWidgetClass *widget_class;

        object_class = G_OBJECT_CLASS (klass);

        object_class->set_property = mb_panel_scaling_image2_set_property;
        object_class->get_property = mb_panel_scaling_image2_get_property;
        object_class->dispose      = mb_panel_scaling_image2_dispose;
        object_class->finalize     = mb_panel_scaling_image2_finalize;

        widget_class = GTK_WIDGET_CLASS (klass);
        widget_class->size_allocate  = mb_panel_scaling_image2_size_allocate;
        widget_class->realize        = mb_panel_scaling_image2_realize;
        widget_class->screen_changed = mb_panel_scaling_image2_screen_changed;
        /* TODO: respect orientation and switch to height-for-width in vertical mode */
        widget_class->get_preferred_width = mb_panel_scaling_image2_get_preferred_width;
        widget_class->draw = mb_panel_scaling_image2_draw;

        g_type_class_add_private (klass, sizeof (MBPanelScalingImage2Private));

        g_object_class_install_property
                (object_class,
                 PROP_ORIENTATION,
                 g_param_spec_enum
                         ("orientation",
                          "orientation",
                          "The containing panels orientation.",
                          GTK_TYPE_ORIENTATION,
                          GTK_ORIENTATION_HORIZONTAL,
                          G_PARAM_READWRITE |
                          G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK |
                          G_PARAM_STATIC_BLURB));

        g_object_class_install_property
                (object_class,
                 PROP_ICON,
                 g_param_spec_string
                         ("icon",
                          "icon",
                          "The loaded icon.",
                          NULL,
                          G_PARAM_READWRITE |
                          G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK |
                          G_PARAM_STATIC_BLURB));
}

/**
 * mb_panel_scaling_image2_new
 * @orientation: The orientation of the containing panel.
 * @icon: The icon to display. This can be an absolute path, or a name
 * of an icon theme icon.
 *
 * Return value: A new #MBPanelScalingImage2 object displaying @icon.
 **/
GtkWidget *
mb_panel_scaling_image2_new (GtkOrientation orientation,
                            const char    *icon)
{
        return g_object_new (MB_PANEL_TYPE_SCALING_IMAGE2,
                             "orientation", orientation,
                             "icon", icon,
                             NULL);
}

/**
 * mb_panel_scaling_image2_set_icon
 * @image: A #MBPanelScalingImage2
 * @icon: The icon to display. This can be an absolute path, or a name
 * of an icon theme icon.
 *
 * Displays @icon in @image.
 **/
void
mb_panel_scaling_image2_set_icon (MBPanelScalingImage2 *image,
                                 const char          *icon)
{
        g_return_if_fail (MB_PANEL_IS_SCALING_IMAGE2 (image));

        g_free (image->priv->icon);
        image->priv->icon = g_strdup (icon);

        if (!gtk_widget_get_realized (GTK_WIDGET (image)))
                return;

        reload_icon (image, TRUE);
}

/**
 * mb_panel_scaling_image2_get_icon
 * @image: A #MBPanelScalingImage2
 *
 * Return value: The displayed icon. This string should not be freed.
 **/
const char *
mb_panel_scaling_image2_get_icon (MBPanelScalingImage2 *image)
{
        g_return_val_if_fail (MB_PANEL_IS_SCALING_IMAGE2 (image), NULL);

        return image->priv->icon;
}
