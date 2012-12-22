/* 
 * Author: Manuel Teira <manuel.teira@telefonica.net>
 *
 * Licensed under the GPL v2 or greater.
 */

#include <config.h>
#include <string.h>
#include <errno.h>
#include <matchbox-panel/mb-panel.h>
#include <gtk/gtk.h>

typedef struct {
        GtkWidget *scale;
        gint32 current_brightness;
        gint32 max_brightness;
        gchar *sysfs_mbn;
        gchar *sysfs_bn;
} BrightnessApplet;

static gint32 sysfs_value_get(const gchar *path)
{
        FILE *fd;
        if ((fd = fopen(path, "r")) == NULL) {
                g_debug("Error in sysfs_value_get for path %s: %s",
                        path, g_strerror(errno));
                return -1;
        }
        gint32 value;
        (void)fscanf(fd, "%d", &value);
        fclose(fd);
        return value;
}

static gboolean sysfs_value_set(const gchar *path, gint32 value)
{
        FILE *fd;
        if ((fd = fopen(path, "w")) == NULL) {
                g_debug ("Error in sysfs_value_set for path %s: %s",
                        path, g_strerror(errno));
                return FALSE;
        }
        fprintf(fd, "%d", value);
        fclose(fd);
        return TRUE;
}

/* Applet destroyed */
static void
brightness_applet_free(BrightnessApplet *applet)
{
        g_free(applet->sysfs_mbn);
        g_free(applet->sysfs_bn);
        g_slice_free(BrightnessApplet, applet);
}

static void
brightness_changed_cb(GtkScaleButton *scale,
                      gdouble val,
                      BrightnessApplet *applet)
{
        gint32 b = (val * applet->max_brightness) / 100;
        if (b != applet->current_brightness) {
                if (sysfs_value_set(applet->sysfs_bn, b)) {
                        applet->current_brightness = b;
                }
        }
}

static gboolean
applet_initialize(BrightnessApplet *applet)
{
        GError *error = NULL;
        gchar *base = g_build_filename("/sys", "class", "backlight", NULL);
        GDir *dir = g_dir_open(base, 0, &error);
        if (dir) {
                const gchar *entry = NULL;
                gchar *path = NULL;
                while ((entry = g_dir_read_name(dir)) != NULL) {
                        path = g_build_filename(base, entry, NULL);

                        if (g_file_test(path, G_FILE_TEST_IS_DIR)) {
                                applet->sysfs_bn = 
                                        g_build_filename(path, 
                                                         "brightness", NULL);
                                applet->sysfs_mbn =
                                        g_build_filename(path, 
                                                         "max_brightness", NULL);

                                if (g_file_test(applet->sysfs_bn, 
                                                G_FILE_TEST_IS_REGULAR) &&
                                    g_file_test(applet->sysfs_mbn, 
                                                G_FILE_TEST_IS_REGULAR)) {

                                        break;
                                } else {
                                        g_free(applet->sysfs_bn);
                                        g_free(applet->sysfs_mbn);
                                        applet->sysfs_bn = NULL;
                                        applet->sysfs_mbn = NULL;
                                }
                        }
                        g_free(path);
                }
                g_dir_close(dir);
        } else {
                g_error("Error opening directory %s", base);
        }
        
        if (applet->sysfs_bn && applet->sysfs_mbn) {
                applet->max_brightness = sysfs_value_get(applet->sysfs_mbn);
                applet->current_brightness = sysfs_value_get(applet->sysfs_bn);
                return TRUE;
        } else {
                return FALSE;
        }
}

G_MODULE_EXPORT GtkWidget *
mb_panel_applet_create(const char *id, GtkOrientation orientation)
{
        BrightnessApplet *applet;
        GtkWidget *scale;
        GtkIconTheme *theme;
        static const gchar *icons[] = {
            "brightness-min",
            "brightness-max",
            "brightness-min",
            "brightness-medium",
            "brightness-max",
            NULL};
            
        theme = gtk_icon_theme_get_default();
        gtk_icon_theme_append_search_path(theme, DATADIR);
            
        applet = g_slice_new(BrightnessApplet);
        scale = gtk_scale_button_new(GTK_ICON_SIZE_BUTTON,
                                     1, 100, 1,
                                     icons);
        applet->scale = scale;
        gtk_widget_set_name(scale, "MatchboxPanelBrightness");

        if (!applet_initialize(applet)) {
            g_error("Error initializing applet");
            brightness_applet_free(applet);
            return NULL;
        }
        gdouble val = (100 * applet->current_brightness) / 
                applet->max_brightness;
        gtk_scale_button_set_value(GTK_SCALE_BUTTON(scale), val);
        g_signal_connect(scale,
                         "value-changed",
                         G_CALLBACK(brightness_changed_cb),
                         applet);
        g_object_weak_ref(G_OBJECT(scale),
                          (GWeakNotify) brightness_applet_free,
                          applet);

        gtk_widget_show(scale);

        return scale;
}
