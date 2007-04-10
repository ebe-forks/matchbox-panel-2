/* 
 * (C) 2006 OpenedHand Ltd.
 *
 * Author: Jorn Baayen <jorn@openedhand.com>
 *
 * Licensed under the GPL v2 or greater.
 */

#include <glib/gi18n.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkwindow.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkalignment.h>

#include <gdk/gdkx.h>

#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>


#define DEFAULT_HEIGHT 32 /* Default panel height */

static GList *open_modules = NULL; /* List of open modules */

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
        
        /* See if we can find a module with this name */
        path = g_module_build_path (applet_path, name);
        if (!g_file_test (path, G_FILE_TEST_EXISTS)) {
                /* No */
                g_warning ("Failed to find applet \"%s\"", name);

                g_free (path);

                return NULL;
        }

        /* Yes: Open it */
        module = g_module_open (path, G_MODULE_BIND_LOCAL);
        if (!module) {
                g_warning ("Failed to load applet \"%s\" (%s).", name, g_module_error());

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
                char **bits;
                GtkWidget *applet;

                bits = g_strsplit (applets[i], ":", 2);

                applet = load_applet (bits[0],
                                      bits[1],
                                      orientation);
                if (applet)
                        pack_func (box, applet, FALSE, FALSE, 0);

                g_strfreev (bits);
        }

        g_strfreev (applets);
}

int
main (int argc, char **argv)
{
        GOptionContext *option_context;
        GOptionGroup *option_group;
        GError *error;
        char *geometry = NULL, *start_applets = NULL, *end_applets = NULL;
        GtkWidget *window, *box, *align;
        int panel_width, panel_height;
        GtkOrientation orientation;
	gboolean want_titlebar = FALSE;

        GOptionEntry option_entries[] = {
                { "geometry", 0, 0, G_OPTION_ARG_STRING, &geometry,
                  "Panel geometry", "[WIDTH][xHEIGHT][{+-}X[{+-}Y]]" },
                { "start-applets", 0, 0, G_OPTION_ARG_STRING, &start_applets,
                  "Applets to pack at the start", "APPLET[:APPLET_ID] ..." },
                { "end-applets", 0, 0, G_OPTION_ARG_STRING, &end_applets,
                  "Applets to pack at the end", "APPLET[:APPLET_ID] ..." },
                { "titlebar", 0, 0, G_OPTION_ARG_NONE, &want_titlebar,
                  "Display in window titlebar (If Matchbox theme supports)", NULL },
                { NULL }
        };

        /* Make sure that GModule is supported */
        if (!g_module_supported ()) {
                g_warning (_("gmodule support not found. gmodule support is "
                             "required for matchbox-panel to work"));
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

        /* Set app name */
        g_set_application_name (_("Matchbox Panel"));

        /* Create window */
        window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

        gtk_widget_set_name (window, "MatchboxPanel");
        
        gtk_window_set_type_hint (GTK_WINDOW (window),
                                  GDK_WINDOW_TYPE_HINT_DOCK);

        /* Set default panel height 	*/
        gtk_window_set_default_size (GTK_WINDOW (window),
                                     -1,
                                     DEFAULT_HEIGHT);

        /* Parse geometry string */
        if (geometry) {
                if (!gtk_window_parse_geometry (GTK_WINDOW (window),
                                                geometry)) {
                        g_warning ("Failed to parse geometry string");

                        gtk_widget_destroy (window);

                        return 1;
                }
        }

        /* Determine window size */
        gtk_window_get_size (GTK_WINDOW (window), &panel_width, &panel_height);

        /* Force size */
        gtk_widget_set_size_request (window, panel_width, panel_height);
        gtk_window_resize (GTK_WINDOW (window), panel_width, panel_height);

        /* Is this a horizontal or a vertical layout? */
        if (panel_width >= panel_height) {
                orientation = GTK_ORIENTATION_HORIZONTAL;
		
		align =  gtk_alignment_new (0.0, 0.0, 1.0, 1.0);
		/* FIXME: Padding should be set from theme? */
		gtk_alignment_set_padding (GTK_ALIGNMENT (align),
					   0, 0, 8, 8);

                box = gtk_hbox_new (FALSE, 0);
        } else {
                orientation = GTK_ORIENTATION_VERTICAL;

		align = gtk_alignment_new (0.0, 0.0, 1.0, 1.0);
		/* FIXME: Padding should be set from theme? */
		gtk_alignment_set_padding (GTK_ALIGNMENT (align),
					   8, 8, 0, 0);
                
                box = gtk_vbox_new (FALSE, 0);
        }

        gtk_container_add (GTK_CONTAINER (window), align);
        gtk_container_add (GTK_CONTAINER (align), box);

	if (want_titlebar)
	  {
	    Atom atoms[3];

	    atoms[0] = XInternAtom (GDK_DISPLAY(), 
				    "_MB_WM_STATE", False);
	    atoms[1] = XInternAtom (GDK_DISPLAY(), 
				    "_MB_WM_STATE_DOCK_TITLEBAR", False);
	    atoms[2] = XInternAtom (GDK_DISPLAY(),
				    "_MB_DOCK_TITLEBAR_SHOW_ON_DESKTOP", False);
	    gtk_widget_realize (window);

	    XChangeProperty(GDK_DISPLAY(), 
			    GDK_WINDOW_XID(window->window), 
			    atoms[0], XA_ATOM, 32,
			    PropModeReplace, (unsigned char *) &atoms[1], 2);
	  }

        gtk_widget_show_all (align);

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
