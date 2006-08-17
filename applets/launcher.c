/* 
 * (C) 2006 OpenedHand Ltd.
 *
 * Author: Jorn Baayen <jorn@openedhand.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <config.h>
#include <gtk/gtkimage.h>
#include <gtk/gtkbutton.h>
#include <gdk/gdkx.h>
#include <string.h>
#include <unistd.h>
#include <matchbox-panel/mb-panel.h>

#ifdef USE_LIBSN
  #define SN_API_NOT_YET_FROZEN 1
  #include <libsn/sn.h>
#endif

typedef struct {
        gboolean use_sn;

        char *name;
        char **argv;
} LauncherData;

static void
launcher_data_free (LauncherData *data)
{
        g_free (data->name);
        g_strfreev (data->argv);

        g_slice_free (LauncherData, data);
}

#define MAX_ARGS 255

/* Convert command line to argv array, stripping % conversions on the way */
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

/* Button clicked */
static void
clicked_cb (GtkButton    *button,
            LauncherData *data)
{
        pid_t child_pid = 0;
#ifdef USE_LIBSN
        SnLauncherContext *context;

        context = NULL;

        if (data->use_sn) {
                SnDisplay *sn_dpy;
                Display *display;
                int screen;

                display = gdk_x11_display_get_xdisplay
                                (gtk_widget_get_display (GTK_WIDGET (button)));
                sn_dpy = sn_display_new (display, NULL, NULL);

                screen = gdk_screen_get_number
                                (gtk_widget_get_screen (GTK_WIDGET (button)));
                context = sn_launcher_context_new (sn_dpy, screen);
                sn_display_unref (sn_dpy);
          
                sn_launcher_context_set_name (context, data->name);
                sn_launcher_context_set_binary_name (context,
                                                     data->argv[0]);
          
                sn_launcher_context_initiate (context,
                                              "matchbox-panel",
                                              data->argv[0],
                                              CurrentTime);
        }
#endif

        switch ((child_pid = fork ())) {
        case -1:
                g_warning ("Fork failed");
                break;
        case 0:
#ifdef USE_LIBSN
                if (data->use_sn)
                        sn_launcher_context_setup_child_process (context);
#endif
                execvp (data->argv[0], data->argv);

                g_warning ("Failed to execvp() %s", data->argv[0]);
                _exit (1);

                break;
        }

#ifdef USE_LIBSN
        if (data->use_sn)
                sn_launcher_context_unref (context);
#endif
}

G_MODULE_EXPORT GtkWidget *
mb_panel_applet_create (const char *id,
                        int         panel_width,
                        int         panel_height)
{
        char *filename;
        GKeyFile *key_file;
        GtkWidget *button, *image;
        GError *error;
        char *icon, *exec, *name;
        gboolean use_sn;
        int shortest_side;
        LauncherData *data;
        
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
        if (!icon) {
                /* An error occured */
                g_warning ("%s", error->message);

                g_error_free (error);

                g_key_file_free (key_file);

                return NULL;
        }

        icon = g_strstrip (icon);

        /* Exec */
        error = NULL;
        exec = g_key_file_get_string (key_file,
                                      "Desktop Entry",
                                      "Exec",
                                      &error);
        if (!exec) {
                /* An error occured */
                g_warning ("%s", error->message);
                
                g_error_free (error);

                g_free (icon);
                g_key_file_free (key_file);

                return NULL;
        }

        exec = g_strstrip (exec);

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

        /* Create image widget */
        image = gtk_image_new ();

        /* Load icon */
        shortest_side = MIN (panel_width, panel_height);

        if (icon[0] == '/') {
                /* Absolute path specified: load icon directly */
                GdkPixbuf *pixbuf;

                error = NULL;
                pixbuf = gdk_pixbuf_new_from_file_at_scale (icon,
                                                            shortest_side,
                                                            shortest_side,
                                                            TRUE,
                                                            &error);
                if (!pixbuf) {
                        /* An error occured */
                        g_warning ("%s", error->message);

                        g_error_free (error);
                        
                        g_free (icon);
                        g_free (exec);
                        g_free (name);
                        gtk_widget_destroy (image);

                        return NULL;
                }

                gtk_image_set_from_pixbuf (GTK_IMAGE (image), pixbuf);
                g_object_unref (pixbuf);
        } else {
                /* No absolute path given. Load icon from icon theme */
                char *p;

                /* Strip extension, so that we can feed the filename to
                 * GTK+'s icon name handling. */
                p = strrchr (icon, '.');
                if (p)
                        *p = 0;

                gtk_image_set_pixel_size (GTK_IMAGE (image), shortest_side);
                gtk_image_set_from_icon_name (GTK_IMAGE (image),
                                              icon,
                                              0);
        }
        
        g_free (icon);

        /* Create button */
        button = gtk_button_new ();

        gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
        GTK_WIDGET_UNSET_FLAGS (button, GTK_CAN_FOCUS);

        /* Strip %-conversions from exec */

        /* Connect to "clicked" signal */
        data = g_slice_new (LauncherData);

        data->use_sn = use_sn;

        data->name = name;

        data->argv = exec_to_argv (exec);
        g_free (exec);
        
        g_signal_connect_data (button,
                               "clicked",
                               G_CALLBACK (clicked_cb),
                               data,
                               (GClosureNotify) launcher_data_free,
                               0);

        /* Add image to button */
        gtk_container_add (GTK_CONTAINER (button), image);

        /* Show! */
        gtk_widget_show_all (button);

        return button;
};
