/* 
 * (C) 2006, 2007 OpenedHand Ltd.
 *
 * Author: Jorn Baayen <jorn@openedhand.com>
 *
 * Licensed under the GPL v2 or greater.
 */

#include <gtk/gtkicontheme.h>
#include <string.h>

#include "mb-panel-scaling-image.h"

struct _MBPanelScalingImagePrivate {
        GtkOrientation orientation;

        char *icon;

        int size;

        gboolean caching;

        GHashTable *cache;

        GtkIconTheme *icon_theme;
        guint icon_theme_changed_id;
};

enum {
        PROP_0,
        PROP_ORIENTATION,
        PROP_ICON,
        PROP_CACHING
};

G_DEFINE_TYPE (MBPanelScalingImage,
               mb_panel_scaling_image,
               GTK_TYPE_IMAGE);

static void
mb_panel_scaling_image_init (MBPanelScalingImage *image)
{
        image->priv = G_TYPE_INSTANCE_GET_PRIVATE (image,
                                                   MB_PANEL_TYPE_SCALING_IMAGE,
                                                   MBPanelScalingImagePrivate);

        image->priv->orientation = GTK_ORIENTATION_HORIZONTAL;

        image->priv->icon = NULL;

        image->priv->caching = FALSE;

        image->priv->cache = NULL;
}

static void
mb_panel_scaling_image_set_property (GObject      *object,
                                     guint         property_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
        MBPanelScalingImage *image;

        image = MB_PANEL_SCALING_IMAGE (object);

        switch (property_id) {
        case PROP_ORIENTATION:
                image->priv->orientation = g_value_get_enum (value);
                break;
        case PROP_ICON:
                mb_panel_scaling_image_set_icon (image,
                                                 g_value_get_string (value));
                break;
        case PROP_CACHING:
                mb_panel_scaling_image_set_caching
                        (image, g_value_get_boolean (value));
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
                break;
        }
}

static void
mb_panel_scaling_image_get_property (GObject    *object,
                                     guint       property_id,
                                     GValue     *value,
                                     GParamSpec *pspec)
{
        MBPanelScalingImage *image;

        image = MB_PANEL_SCALING_IMAGE (object);

        switch (property_id) {
        case PROP_ORIENTATION:
                g_value_set_enum (value, image->priv->orientation);
                break;
        case PROP_ICON:
                g_value_set_string (value, image->priv->icon);
                break;
        case PROP_CACHING:
                g_value_set_boolean (value, image->priv->caching);
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
                break;
        }
}

static void
mb_panel_scaling_image_dispose (GObject *object)
{
        MBPanelScalingImage *image;
        GObjectClass *object_class;

        image = MB_PANEL_SCALING_IMAGE (object);

        if (image->priv->icon_theme_changed_id) {
                g_signal_handler_disconnect
                        (image->priv->icon_theme,
                         image->priv->icon_theme_changed_id);
                image->priv->icon_theme_changed_id = 0;
        }

        if (image->priv->cache)
                g_hash_table_destroy (image->priv->cache);

        object_class = G_OBJECT_CLASS (mb_panel_scaling_image_parent_class);
        object_class->dispose (object);
}

static void
mb_panel_scaling_image_finalize (GObject *object)
{
        MBPanelScalingImage *image;
        GObjectClass *object_class;

        image = MB_PANEL_SCALING_IMAGE (object);

        g_free (image->priv->icon);

        object_class = G_OBJECT_CLASS (mb_panel_scaling_image_parent_class);
        object_class->finalize (object);
}

static gboolean
return_true (gpointer a,
             gpointer b,
             gpointer c)
{
        return TRUE;
}

/* Clear the pixbuf cache */
static void
clear_cache (MBPanelScalingImage *image)
{
        if (!image->priv->caching)
                return;
        
        g_hash_table_foreach_remove (image->priv->cache,
                                     return_true,
                                     NULL);
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
reload_icon (MBPanelScalingImage *image, gboolean force)
{
        int size;
        char *file;
        GdkPixbuf *pixbuf;
        GError *error;

        /* Determine the required icon size */
        if (image->priv->orientation == GTK_ORIENTATION_HORIZONTAL)
                size = GTK_WIDGET (image)->allocation.height;
        else
                size = GTK_WIDGET (image)->allocation.width;

        if (!force && size == image->priv->size) {
                return;
        }

        image->priv->size = size;

        if (!image->priv->icon) {
                gtk_image_set_from_pixbuf (GTK_IMAGE (image), NULL);

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
        pixbuf = gdk_pixbuf_new_from_file_at_scale (file,
                                                    size,
                                                    size,
                                                    TRUE,
                                                    &error);

        g_free (file);

        if (pixbuf) {
                gtk_image_set_from_pixbuf (GTK_IMAGE (image), pixbuf);

                if (image->priv->caching) {
                        g_hash_table_insert (image->priv->cache,
                                             image->priv->icon,
                                             pixbuf);
                } else
                        g_object_unref (pixbuf);
        } else {
                g_warning ("No pixbuf found: %s", error->message);

                g_error_free (error);
        }
}

/* Icon theme changed */
static void
icon_theme_changed_cb (GtkIconTheme   *icon_theme,
                       MBPanelScalingImage *image)
{
        if (!GTK_WIDGET_REALIZED (image))
                return;

        clear_cache (image);

        reload_icon (image, TRUE);
}

static void
mb_panel_scaling_image_size_allocate (GtkWidget *widget,
                                      GtkAllocation *allocation)
{
        MBPanelScalingImage *image = MB_PANEL_SCALING_IMAGE (widget);
        GtkWidgetClass *widget_class;

        widget_class = GTK_WIDGET_CLASS (mb_panel_scaling_image_parent_class);
        widget_class->size_allocate (widget, allocation);
        
        reload_icon (image, FALSE);
}

static void
mb_panel_scaling_image_realize (GtkWidget *widget)
{
        GtkWidgetClass *widget_class;

        reload_icon (MB_PANEL_SCALING_IMAGE (widget), TRUE);
        
        widget_class = GTK_WIDGET_CLASS (mb_panel_scaling_image_parent_class);
        widget_class->realize (widget);
}

static void
mb_panel_scaling_image_unrealize (GtkWidget *widget)
{
        GtkWidgetClass *widget_class;

        clear_cache (MB_PANEL_SCALING_IMAGE (widget));

        widget_class = GTK_WIDGET_CLASS (mb_panel_scaling_image_parent_class);
        widget_class->unrealize (widget);
}

static void
mb_panel_scaling_image_screen_changed (GtkWidget *widget,
                                       GdkScreen *old_screen)
{
        MBPanelScalingImage *image;
        GtkWidgetClass *widget_class;

        GdkScreen *screen;
        GtkIconTheme *new_icon_theme;

        image = MB_PANEL_SCALING_IMAGE (widget);

        /* Get associated icon theme */
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

        /* Reload icon if we are realized */
        if (GTK_WIDGET_REALIZED (widget)) {
                clear_cache (image);

                reload_icon (MB_PANEL_SCALING_IMAGE (widget), TRUE);
        }
        
        widget_class = GTK_WIDGET_CLASS (mb_panel_scaling_image_parent_class);
        widget_class->screen_changed (widget, old_screen);
}

static void
mb_panel_scaling_image_class_init (MBPanelScalingImageClass *klass)
{
        GObjectClass *object_class;
        GtkWidgetClass *widget_class;

        object_class = G_OBJECT_CLASS (klass);

        object_class->set_property = mb_panel_scaling_image_set_property;
        object_class->get_property = mb_panel_scaling_image_get_property;
        object_class->dispose      = mb_panel_scaling_image_dispose;
        object_class->finalize     = mb_panel_scaling_image_finalize;

        widget_class = GTK_WIDGET_CLASS (klass);

        widget_class->size_allocate  = mb_panel_scaling_image_size_allocate;
        widget_class->realize        = mb_panel_scaling_image_realize;
        widget_class->unrealize      = mb_panel_scaling_image_unrealize;
        widget_class->screen_changed = mb_panel_scaling_image_screen_changed;

        g_type_class_add_private (klass, sizeof (MBPanelScalingImagePrivate));

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

        g_object_class_install_property
                (object_class,
                 PROP_CACHING,
                 g_param_spec_boolean
                         ("caching",
                          "Caching.",
                          "TRUE if icons should be cached.",
                          FALSE,
                          G_PARAM_READWRITE |
                          G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK |
                          G_PARAM_STATIC_BLURB));
}

/**
 * mb_panel_scaling_image_new
 * @orientation: The orientation of the containing panel.
 * @icon: The icon to display. This can be an absolute path, or a name
 * of an icon theme icon.
 *
 * Return value: A new #MBPanelScalingImage object displaying @icon.
 **/
GtkWidget *
mb_panel_scaling_image_new (GtkOrientation orientation,
                            const char    *icon)
{
        return g_object_new (MB_PANEL_TYPE_SCALING_IMAGE,
                             "orientation", orientation,
                             "icon", icon,
                             NULL);
}

/**
 * mb_panel_scaling_image_set_icon
 * @image: A #MBPanelScalingImage
 * @icon: The icon to display. This can be an absolute path, or a name
 * of an icon theme icon.
 *
 * Displays @icon in @image.
 **/
void
mb_panel_scaling_image_set_icon (MBPanelScalingImage *image,
                                 const char          *icon)
{
        g_return_if_fail (MB_PANEL_IS_SCALING_IMAGE (image));

        g_free (image->priv->icon);
        if (icon)
                image->priv->icon = g_strdup (icon);

        if (!GTK_WIDGET_REALIZED (image))
                return;

        if (image->priv->caching) {
                /* Check cache first */
                GdkPixbuf *pixbuf;

                pixbuf = g_hash_table_lookup (image->priv->cache,
                                              icon);

                if (pixbuf != NULL) {
                        gtk_image_set_from_pixbuf (GTK_IMAGE (image), pixbuf);

                        return;
                }
        }

        reload_icon (image, TRUE);
}

/**
 * mb_panel_scaling_image_get_icon
 * @image: A #MBPanelScalingImage
 *
 * Return value: The displayed icon. This string should not be freed.
 **/
const char *
mb_panel_scaling_image_get_icon (MBPanelScalingImage *image)
{
        g_return_val_if_fail (MB_PANEL_IS_SCALING_IMAGE (image), NULL);
        
        return image->priv->icon;
}

/**
 * mb_panel_scaling_image_set_caching
 * @image: A #MBPanelScalingImage
 * @caching: TRUE if image caching is desired
 *
 * Sets the caching mode to @caching. If @caching is TRUE, 
 * #GdkPixbuf objects previously loaded by @image will be kept around.
 * Use this if you want to quickly alternate between a fixed set of images.
 *
 * Setting @caching to FALSE causes the cache to be purged.
 **/
void
mb_panel_scaling_image_set_caching (MBPanelScalingImage *image,
                                    gboolean             caching)
{
        g_return_if_fail (MB_PANEL_IS_SCALING_IMAGE (image));

        if (image->priv->caching == caching)
                return;

        image->priv->caching = caching;

        if (caching) {
                /* Create cache */
                image->priv->cache = g_hash_table_new_full (g_str_hash,
                                                            g_str_equal,
                                                            g_free,
                                                            g_object_unref);
        } else {
                /* Destroy cache */
                g_hash_table_destroy (image->priv->cache);
                image->priv->cache = NULL;
        }
}

/**
 * mb_panel_scaling_image_get_caching
 * @image: A #MBPanelScalingImage
 *
 * Return value: TRUE if image caching is enabled.
 **/
gboolean
mb_panel_scaling_image_get_caching (MBPanelScalingImage *image)
{
        g_return_val_if_fail (MB_PANEL_IS_SCALING_IMAGE (image), FALSE);
        
        return image->priv->caching;
}
