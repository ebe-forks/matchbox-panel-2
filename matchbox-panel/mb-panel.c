/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 * (C) 2006-2013 Intel Corp
 *
 * Authors: Ross Burton <ross.burton@intel.com>
 *          Jorn Baayen <jorn@openedhand.com>
 *
 * Licensed under the GPL v2 or greater.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>

#include <X11/Xatom.h>

#define DEFAULT_HEIGHT 32 /* Default panel height */
#define PADDING        4  /* Applet padding */

static GList *open_modules = NULL; /* List of open modules */

enum {
        ATOM_WM_STRUT_PARTIAL,
        ATOM_MB_STATE,
        ATOM_STATE_DOCT_TITLEBAR,
        ATOM_DOCK_TITLEBAR_DESKTOP,
        /* Counter */
        ATOM_LAST
};

static Atom atoms[ATOM_LAST];

/* Load applet @name with ID @id */
static GtkWidget *
load_applet (const char    *name,
             const char    *id,
             GtkOrientation orientation)
{
        const char *applet_path;
        char *path;
        GModule *module;
        GtkWidget * (* create_func) (const char    *id,
                                     GtkOrientation orientation);

        /* Get MATCHBOX_PANEL_APPLET_PATH */
        applet_path = g_getenv ("MATCHBOX_PANEL_APPLET_PATH");
        if (!applet_path)
                applet_path = DEFAULT_APPLET_PATH;

        /* Create the path to the applet */
        path = g_module_build_path (applet_path, name);

        /* Try and open it */
        module = g_module_open (path, G_MODULE_BIND_LOCAL | G_MODULE_BIND_LAZY);
        if (!module) {
                g_warning ("Failed to load applet \"%s\" (%s).",
                           name, g_module_error ());

                g_free (path);

                return NULL;
        }

        g_free (path);

        /* Dig up the mb_panel_applet_create symbol */
        if (!g_module_symbol (module,
                              "mb_panel_applet_create",
                              (gpointer) &create_func)) {
                g_warning ("Applet \"%s\" does not export the "
                           "\"mb_panel_applet_create\" symbol.", name);

                g_module_close (module);

                return NULL;
        }

        /* Keep track of open modules */
        open_modules = g_list_prepend (open_modules, module);

        /* Run mb_panel_applet_create */
        return create_func (id, orientation);
}

/* Load the applets from @applets_desc into @box, packing them using
 * @pack_func */
static void
load_applets (const char    *applets_desc,
              GtkBox        *box,
              void        ( *pack_func) (GtkBox    *box,
                                         GtkWidget *widget,
                                         gboolean   expand,
                                         gboolean   fill,
                                         guint      padding),
              GtkOrientation orientation)
{
        char **applets;
        int i;

        /* Check for NULL description */
        if (!applets_desc)
                return;

        /* Parse description */
        applets = g_strsplit (applets_desc, ",", -1);

        for (i = 0; applets[i]; i++) {
                char *s;
                char **bits;
                GtkWidget *applet;

                s = applets[i];
                if (s == NULL || s[0] == '\0')
                        continue;

                bits = g_strsplit (s, ":", 2);

                applet = load_applet (bits[0],
                                      bits[1],
                                      orientation);
                if (applet)
                        pack_func (box, applet, FALSE, FALSE, PADDING);

                g_strfreev (bits);
        }

        g_strfreev (applets);
}

static void
set_struts (GtkWidget *window, GtkPositionType edge, int size)
{
        Display *xdisplay;
        int x, y, w, h;
        struct {
                long left;
                long right;
                long top;
                long bottom;
                long left_start_y;
                long left_end_y;
                long right_start_y;
                long right_end_y;
                long top_start_x;
                long top_end_x;
                long bottom_start_x;
                long bottom_end_x;
        } struts = { 0, };

        xdisplay = GDK_SCREEN_XDISPLAY (gtk_widget_get_screen (window));
        g_assert (xdisplay);

        gtk_window_get_position (GTK_WINDOW (window), &x, &y);
        gtk_window_get_size (GTK_WINDOW (window), &w, &h);

        switch (edge) {
        case GTK_POS_LEFT:
                struts.left = size;
                struts.left_start_y = y;
                struts.left_end_y = y + h;
                break;
        case GTK_POS_RIGHT:
                struts.right = size;
                struts.right_start_y = y;
                struts.right_end_y = y + h;
                break;
        case GTK_POS_TOP:
                struts.top = size;
                struts.top_start_x = x;
                struts.top_end_x = x + w;
                break;
        case GTK_POS_BOTTOM:
                struts.bottom = size;
                struts.bottom_start_x = x;
                struts.bottom_end_x = x + w;
                break;
        }

        gdk_error_trap_push ();
        XChangeProperty (xdisplay,
                         GDK_WINDOW_XID (gtk_widget_get_window (window)),
                         atoms[ATOM_WM_STRUT_PARTIAL], XA_CARDINAL, 32,
                         PropModeReplace,
                         (guchar *) &struts, 12);
        gdk_error_trap_pop_ignored ();
}

#if 0
static void
screen_size_changed_cb (GdkScreen *screen, GtkWidget *window)
{
        gint       x, y, w, h;
        gint       screen_width, screen_height;
        gtk_window_get_position (GTK_WINDOW (window), &x, &y);
        gtk_window_get_size (GTK_WINDOW (window), &w, &h);

        screen_width  = gdk_screen_get_width (screen);
        screen_height = gdk_screen_get_height (screen);

        if (snap_right)
                x = screen_width - w;

        if (snap_bottom)
                y = screen_height - h;

        if (center_horizontal)
                x = (screen_width - w) / 2;

        if (center_vertical)
                y = (screen_height - h)/ 2;

        if (fullscreen)
                {
                        if (w > h)
                                {
                                        w = screen_width;
                                        x = 0;
                                }
                        else
                                {
                                        h = screen_height;
                                        y = 0;
                                }

                        gtk_window_move (GTK_WINDOW (window), x, y);
                        gtk_widget_set_size_request (window, w, h);
                        gtk_window_resize (GTK_WINDOW (window), w, h);
                }
        else if (snap_right || snap_bottom ||
                 center_horizontal || center_vertical)
                {
                        gtk_window_move (GTK_WINDOW (window), x, y);
                }

        set_struts (window, x, y, w, h);
}
#endif

static void
get_atoms (Display *xdisplay)
{
        static const char *names[] = {
                "_NET_WM_STRUT_PARTIAL",
                "_MB_WM_STATE",
                "_MB_WM_STATE_DOCK_TITLEBAR",
                "_MB_DOCK_TITLEBAR_SHOW_ON_DESKTOP"
        };

        XInternAtoms (xdisplay, (char**)names, G_N_ELEMENTS (names), False, atoms);
}

int
main (int argc, char **argv)
{
        GOptionContext *option_context;
        GOptionGroup *option_group;
        GError *error;
        char *start_applets = NULL, *end_applets = NULL;
        char *edge_string = NULL;
        int size = DEFAULT_HEIGHT;
        int screen_num = -1;
        int monitor_num = -1;
        GtkPositionType edge = GTK_POS_TOP;
        GtkWidget *window, *box, *frame;
        GdkDisplay *display;
        GdkScreen *screen;
        GtkOrientation orientation = GTK_ORIENTATION_HORIZONTAL;
        gboolean in_titlebar = FALSE;
        GdkRectangle screen_geom;

        /* TODO: add these as groups (applets / position) */
        GOptionEntry option_entries[] = {
                { "start-applets", 0, 0, G_OPTION_ARG_STRING, &start_applets,
                  N_("Applets to pack at the start"), N_("APPLET[:APPLET_ID] ...") },
                { "end-applets", 0, 0, G_OPTION_ARG_STRING, &end_applets,
                  N_("Applets to pack at the end"), N_("APPLET[:APPLET_ID] ...") },

                { "screen", 'n', 0, G_OPTION_ARG_INT, &screen_num,
                  N_("Screen number"), N_("SCREEN") },
                { "monitor", 'm', 0, G_OPTION_ARG_INT, &monitor_num,
                  N_("Monitor number"), N_("MONITOR") },

                { "titlebar", 't', 0, G_OPTION_ARG_NONE, &in_titlebar,
                  N_("Display in window titlebar (with Matchbox theme support)"), NULL },
                { "edge", 'e', 0, G_OPTION_ARG_STRING, &edge_string,
                  N_("Panel edge"), N_("TOP|BOTTON|LEFT|RIGHT") },
                { "size", 's', 0, G_OPTION_ARG_INT, &size,
                  N_("Panel size"), N_("PIXELS")},

                { NULL }
        };

        /* Make sure that GModule is supported */
        if (!g_module_supported ()) {
                g_warning (_("GModule support not found, this is required for matchbox-panel to work"));
                return -1;
        }

        /* Set up command line handling */
        option_context = g_option_context_new (NULL);

        option_group = g_option_group_new ("matchbox-panel",
                                           N_("Matchbox Panel"),
                                           N_("Matchbox Panel options"),
                                           NULL, NULL);
        g_option_group_add_entries (option_group, option_entries);
        g_option_context_set_main_group (option_context, option_group);

        g_option_context_add_group (option_context,
                                    gtk_get_option_group (TRUE));

        /* Parse command line */
        error = NULL;
        if (!g_option_context_parse (option_context, &argc, &argv, &error)) {
                g_option_context_free (option_context);

                g_warning ("%s", error->message);
                g_error_free (error);

                return 1;
        }

        g_option_context_free (option_context);

        /* Can't be in the titlebar *and* on an edge, so check for this and exit */
        if (in_titlebar && edge_string) {
                g_printerr ("Cannot specify both --edge and --titlebar\n");
                return 1;
        }

        if (edge_string) {
                if (g_ascii_strcasecmp (edge_string, "top") == 0) {
                        edge = GTK_POS_TOP;
                } else if (g_ascii_strcasecmp (edge_string, "bottom") == 0) {
                        edge = GTK_POS_BOTTOM;
                } else if (g_ascii_strcasecmp (edge_string, "left") == 0) {
                        edge = GTK_POS_LEFT;
                } else if (g_ascii_strcasecmp (edge_string, "right") == 0) {
                        edge = GTK_POS_RIGHT;
                } else {
                        g_printerr ("Unparsable edge '%s', expecting top/bottom/left/right\n", edge_string);
                        return 1;
                }
                g_free (edge_string);
        }

        /* Set app name */
        g_set_application_name (_("Matchbox Panel"));

        display = gdk_display_get_default ();

        get_atoms (GDK_DISPLAY_XDISPLAY (display));

        if (screen_num != -1) {
                screen = gdk_display_get_screen (display, screen_num);
        } else {
                screen = gdk_display_get_default_screen (display);
        }

        if (monitor_num == -1) {
                monitor_num = gdk_screen_get_primary_monitor (screen);
        }

        /* Note that this is bare monitor geometry and not the work area, so
           panels will overlap. */
        gdk_screen_get_monitor_geometry (screen, monitor_num, &screen_geom);

        /* Create window */
        window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
        gtk_widget_set_name (window, "MatchboxPanel");
        gtk_window_set_type_hint (GTK_WINDOW (window), GDK_WINDOW_TYPE_HINT_DOCK);
        gtk_window_set_has_resize_grip (GTK_WINDOW (window), FALSE);

        /* No key focus please */
        gtk_window_set_accept_focus (GTK_WINDOW (window), FALSE);

        gtk_widget_realize (window);

        /* Set size */
        if (!in_titlebar) {
                /* TODO: hook this up to GdkScreen:size-changed */

                /* Orientation and size */
                switch (edge) {
                case GTK_POS_TOP:
                case GTK_POS_BOTTOM:
                        orientation = GTK_ORIENTATION_HORIZONTAL;
                        gtk_widget_set_size_request (window,
                                           screen_geom.width, size);
                        break;
                case GTK_POS_LEFT:
                case GTK_POS_RIGHT:
                        orientation = GTK_ORIENTATION_VERTICAL;
                        gtk_widget_set_size_request (window,
                                           size, screen_geom.height);
                        break;
                }

                /* Position */
                switch (edge) {
                case GTK_POS_TOP:
                case GTK_POS_LEFT:
                        gtk_window_move (GTK_WINDOW (window),
                                         screen_geom.x, screen_geom.y);
                        break;
                case GTK_POS_RIGHT:
                        gtk_window_move (GTK_WINDOW (window),
                                         screen_geom.x + screen_geom.width - size,
                                         screen_geom.y);
                        break;
                case GTK_POS_BOTTOM:
                        gtk_window_move (GTK_WINDOW (window),
                                         screen_geom.x,
                                         screen_geom.y + screen_geom.height - size);
                }

                set_struts (window, edge, size);
        }

        /* Add frame */
        frame = gtk_frame_new (NULL);
        gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);
        gtk_container_add (GTK_CONTAINER (window), frame);
        gtk_widget_show (frame);

        /* Is this a horizontal or a vertical layout? */
        if (orientation == GTK_ORIENTATION_HORIZONTAL) {
                gtk_widget_set_name (frame, "MatchboxPanelFrameHorizontal");
        } else {
                gtk_widget_set_name (frame, "MatchboxPanelFrameVertical");
        }
        box = gtk_box_new (orientation, 0);

        gtk_container_add (GTK_CONTAINER (frame), box);
        gtk_widget_show (box);

#if 0
        /* Do we want to display the panel in the Matchbox titlebar? */
        if (in_titlebar) {
                XChangeProperty (GDK_SCREEN_XDISPLAY (screen),
                                 GDK_WINDOW_XID (gtk_widget_get_window (window)),
                                 atoms[0], XA_ATOM, 32,
                                 PropModeReplace,
                                 (unsigned char *) &atoms[1], 2);
        } else {
                g_signal_connect (screen, "size-changed",
                                  G_CALLBACK (screen_size_changed_cb),
                                  window);
        }
#endif

        /* Load applets */
        load_applets (start_applets,
                      GTK_BOX (box),
                      gtk_box_pack_start,
                      orientation);
        load_applets (end_applets,
                      GTK_BOX (box),
                      gtk_box_pack_end,
                      orientation);

        /* And go! */
        gtk_widget_show (window);

        gtk_main ();

        /* Cleanup */
        gtk_widget_destroy (window);

        while (open_modules) {
                g_module_close (open_modules->data);
                open_modules = g_list_delete_link (open_modules,
                                                   open_modules);
        }

        return 0;
}
