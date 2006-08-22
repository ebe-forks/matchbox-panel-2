/* 
 * (C) 2006 OpenedHand Ltd.
 *
 * Author: Jorn Baayen <jorn@openedhand.com>
 *
 * Licensed under the GPL v2 or greater.
 */

#include <config.h>
#include <gtk/gtkimage.h>
#include <gtk/gtkeventbox.h>
#include <gtk/gtkicontheme.h>
#include <gdk/gdkx.h>
#include <string.h>
#include <unistd.h>
#include <matchbox-panel/mb-panel.h>

#ifdef USE_LIBSN
  #define SN_API_NOT_YET_FROZEN 1
  #include <libsn/sn.h>
#endif

typedef struct {
        GtkImage *image;

        char *icon;
        int icon_size;

        GtkIconTheme *icon_theme;
        guint icon_theme_changed_id;

        gboolean button_down;

        gboolean use_sn;

        char *name;
        char **argv;
} LauncherApplet;

static void
launcher_applet_free (LauncherApplet *applet)
{
        g_free (applet->icon);

        if (applet->icon_theme_changed_id) {
                g_signal_handler_disconnect (applet->icon_theme,
                                             applet->icon_theme_changed_id);
        }

        g_free (applet->name);
        g_strfreev (applet->argv);

        g_slice_free (LauncherApplet, applet);
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
find_icon (LauncherApplet *applet)
{
        GtkIconInfo *info;
        char *icon, *stripped;

        if (g_path_is_absolute (applet->icon)) {
                if (g_file_test (applet->icon, G_FILE_TEST_EXISTS))
                        return g_strdup (applet->icon);
                else
                        icon = g_path_get_basename (applet->icon);
        } else
                icon = applet->icon;

        stripped = strip_extension (icon);

        if (icon != applet->icon)
                g_free (icon);

        info = gtk_icon_theme_lookup_icon (applet->icon_theme, 
                                           stripped,
                                           applet->icon_size,
                                           0);
        
        g_free (stripped);

        if (info) {
                char *file;

                file = g_strdup (gtk_icon_info_get_filename (info));

                gtk_icon_info_free (info);

                return file;
        } else
                return NULL;
}

/* Icon theme changed */
static void
icon_theme_changed_cb (GtkIconTheme   *icon_theme,
                       LauncherApplet *applet)
{
        /* Reload icon */
        char *file;
	GdkPixbuf *pixbuf;
        GError *error;

        file = find_icon (applet);
	if (!file) {
                g_warning ("Icon \"%s\" not found", applet->icon);

                return;
        }

        error = NULL;
        pixbuf = gdk_pixbuf_new_from_file_at_scale (file,
                                                    applet->icon_size,
                                                    applet->icon_size,
                                                    TRUE,
                                                    &error);

        g_free (file);

        if (pixbuf) {
                gtk_image_set_from_pixbuf (applet->image, pixbuf);

                g_object_unref (pixbuf);
        } else {
                g_warning (error->message);

                g_error_free (error);
        }
}

/* Screen set or changed */
static void
screen_changed_cb (GtkWidget      *widget,
                   GdkScreen      *old_screen,
                   LauncherApplet *applet)
{
        GdkScreen *screen;
        GtkIconTheme *new_icon_theme;

        /* Get associated icon theme */
        screen = gtk_widget_get_screen (widget);
        new_icon_theme = gtk_icon_theme_get_for_screen (screen);
        if (applet->icon_theme == new_icon_theme)
                return;

        if (applet->icon_theme_changed_id) {
                g_signal_handler_disconnect (applet->icon_theme,
                                             applet->icon_theme_changed_id);
        }

        applet->icon_theme = new_icon_theme;

        applet->icon_theme_changed_id =
                g_signal_connect (applet->icon_theme,
                                  "changed",
                                  G_CALLBACK (icon_theme_changed_cb),
                                  applet);

        icon_theme_changed_cb (applet->icon_theme, applet);
}

/* Convert command line to argv array, stripping % conversions on the way */
#define MAX_ARGS 255

static char **
exec_to_argv (const char *exec)
{
	const char *p;
        char *buf, *bufp, **argv;
        int nargs;
        gboolean escape, single_quote, double_quote;

        argv = g_new (char *, MAX_ARGS + 1);
        buf = g_alloca (strlen (exec) + 1);
        bufp = buf;
        nargs = 0;
        escape = single_quote = double_quote = FALSE;

	for (p = exec; *p; p++) {
                if (escape) {
                        *bufp++ = *p;

                        escape = FALSE;
                } else {
                        switch (*p) {
                        case '\\':
                                escape = TRUE;
                                
                                break;
                        case '%':
                                /* Strip '%' conversions */
                                if (p[1] && p[1] == '%')
                                        *bufp++ = *p;

                                p++;

                                break;
                        case '\'':
                                if (double_quote)
                                        *bufp++ = *p;
                                else
                                        single_quote = !single_quote;

                                break;
                        case '\"':
                                if (single_quote)
                                        *bufp++ = *p;
                                else
                                        double_quote = !double_quote;

                                break;
                        case ' ':
                                if (single_quote || double_quote)
                                        *bufp++ = *p;
                                else {
                                        *bufp = 0;

                                        if (nargs < MAX_ARGS)
                                                argv[nargs++] = g_strdup (buf);

                                        bufp = buf;
                                }

                                break;
                        default:
                                *bufp++ = *p;
                                break;
                        }
                }
	}

        if (bufp != buf) {
                *bufp = 0;

                if (nargs < MAX_ARGS)
	                argv[nargs++] = g_strdup (buf);
        }

        argv[nargs] = NULL;

        return argv;
}

/* Button pressed on event box */
static gboolean
button_press_event_cb (GtkWidget      *event_box,
                       GdkEventButton *event,
                       LauncherApplet *applet)
{
        if (event->button != 1)
                return TRUE;

        applet->button_down = TRUE;

        return TRUE;
}

/* Button released on event box */
static gboolean
button_release_event_cb (GtkWidget      *event_box,
                         GdkEventButton *event,
                         LauncherApplet *applet)
{
        int x, y;
        pid_t child_pid = 0;
#ifdef USE_LIBSN
        SnLauncherContext *context;
#endif

        if (event->button != 1 || !applet->button_down)
                return TRUE;

        applet->button_down = FALSE;

        /* Only process if the button was released inside the button */
        gtk_widget_translate_coordinates (event_box,
                                          event_box->parent,
                                          event->x,
                                          event->y,
                                          &x,
                                          &y);

        if (x < event_box->allocation.x ||
            x > event_box->allocation.x + event_box->allocation.width ||
            y < event_box->allocation.y ||
            y > event_box->allocation.y + event_box->allocation.height)
                return TRUE;

#ifdef USE_LIBSN
        context = NULL;

        if (applet->use_sn) {
                SnDisplay *sn_dpy;
                Display *display;
                int screen;

                display = gdk_x11_display_get_xdisplay
                              (gtk_widget_get_display (GTK_WIDGET (event_box)));
                sn_dpy = sn_display_new (display, NULL, NULL);

                screen = gdk_screen_get_number
                              (gtk_widget_get_screen (GTK_WIDGET (event_box)));
                context = sn_launcher_context_new (sn_dpy, screen);
                sn_display_unref (sn_dpy);
          
                sn_launcher_context_set_name (context, applet->name);
                sn_launcher_context_set_binary_name (context,
                                                     applet->argv[0]);
          
                sn_launcher_context_initiate (context,
                                              "matchbox-panel",
                                              applet->argv[0],
                                              CurrentTime);
        }
#endif

        switch ((child_pid = fork ())) {
        case -1:
                g_warning ("Fork failed");
                break;
        case 0:
#ifdef USE_LIBSN
                if (applet->use_sn)
                        sn_launcher_context_setup_child_process (context);
#endif
                execvp (applet->argv[0], applet->argv);

                g_warning ("Failed to execvp() %s", applet->argv[0]);
                _exit (1);

                break;
        }

#ifdef USE_LIBSN
        if (applet->use_sn)
                sn_launcher_context_unref (context);
#endif

        return TRUE;
}

/* Someone took or released the GTK+ grab */
static void
grab_notify_cb (GtkWidget      *widget,
                gboolean        was_grabbed,
                LauncherApplet *applet)
{
        if (!was_grabbed) {
                /* It wasn't us. Reset press state */
                applet->button_down = FALSE;
        }
}

G_MODULE_EXPORT GtkWidget *
mb_panel_applet_create (const char *id,
                        int         panel_width,
                        int         panel_height)
{
        char *filename;
        GKeyFile *key_file;
        GtkWidget *event_box, *image;
        GError *error;
        char *icon, *exec, *name;
        gboolean use_sn;
        LauncherApplet *applet;
        
        /* Try to find a .desktop file for @id */
        key_file = g_key_file_new ();

        filename = g_strdup_printf ("applications/%s.desktop", id);
        error = NULL;
        if (!g_key_file_load_from_data_dirs (key_file,
                                             filename,
                                             NULL,
                                             G_KEY_FILE_NONE,
                                             &error)) {
                /* An error occured */
                g_warning ("%s", error->message);
                
                g_error_free (error);

                g_free (filename);
                g_key_file_free (key_file);

                return NULL;
        }

        g_free (filename);

        /* Found and opened keyfile. Read the values we want. */

        /* Icon */
        error = NULL;
        icon = g_key_file_get_string (key_file,
                                      "Desktop Entry",
                                      "Icon",
                                      &error);
        icon = icon ? g_strstrip (icon) : NULL;
        if (!icon || icon[0] == 0) {
                if (error) {
                        g_warning ("%s", error->message);

                        g_error_free (error);
                } else
                        g_warning ("No icon specified");

                g_key_file_free (key_file);

                return NULL;
        }

        /* Exec */
        error = NULL;
        exec = g_key_file_get_string (key_file,
                                      "Desktop Entry",
                                      "Exec",
                                      &error);
        exec = exec ? g_strstrip (exec) : NULL;
        if (!exec || exec[0] == 0) {
                if (error) {
                        g_warning ("%s", error->message);

                        g_error_free (error);
                } else
                        g_warning ("No exec specified");

                g_free (icon);
                g_key_file_free (key_file);

                return NULL;
        }

        /* Name */
        name = g_key_file_get_string (key_file,
                                      "Desktop Entry",
                                      "Name",
                                      NULL);

        /* StartupNotify */
        use_sn = g_key_file_get_boolean (key_file,
                                         "Desktop Entry",
                                         "StartupNotify",
                                         NULL);
        
        /* Close key file */
        g_key_file_free (key_file);

        /* Create widgets */
        event_box = gtk_event_box_new ();

        gtk_widget_set_name (event_box, "MatchboxPanelLauncher");

        image = gtk_image_new ();

        gtk_container_add (GTK_CONTAINER (event_box), image);

        /* Set up applet structure */
        applet = g_slice_new (LauncherApplet);

        applet->image = GTK_IMAGE (image);
        
        applet->icon = icon;
        applet->icon_size = MIN (panel_width, panel_height);

        applet->icon_theme = NULL;
        applet->icon_theme_changed_id = 0;

        applet->button_down = FALSE;

        applet->use_sn = use_sn;

        applet->name = name;

        applet->argv = exec_to_argv (exec);
        g_free (exec);

        g_object_weak_ref (G_OBJECT (event_box),
                           (GWeakNotify) launcher_applet_free,
                           applet);
        
        /* Listen to events */
        g_signal_connect (event_box,
                          "button-press-event",
                          G_CALLBACK (button_press_event_cb),
                          applet);
        g_signal_connect (event_box,
                          "button-release-event",
                          G_CALLBACK (button_release_event_cb),
                          applet);
        g_signal_connect (event_box,
                          "grab-notify",
                          G_CALLBACK (grab_notify_cb),
                          applet);
        g_signal_connect (event_box,
                          "screen-changed",
                          G_CALLBACK (screen_changed_cb),
                          applet);
        
        /* Show! */
        gtk_widget_show_all (event_box);

        return event_box;
};
