/* 
 * (C) 2006 OpenedHand Ltd.
 *
 * Author: Jorn Baayen <jorn@openedhand.com>
 *
 * Licensed under the GPL v2 or greater.
 */

#include <config.h>
#include <gtk/gtkmenubar.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtkimagemenuitem.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkimage.h>
#include <gtk/gtkmain.h>
#include <gdk/gdkx.h>
#include <X11/Xatom.h>
#include <glib/gi18n.h>
#include <matchbox-panel/mb-panel.h>

typedef struct {
        GtkMenuItem *menu_item;
        GtkImage *image;
        GtkMenu *menu;

        Atom stacking_atom;
        Atom utf8_string_atom;
        Atom visible_name_atom;
        Atom name_atom;
        Atom active_window_atom;
        
        GdkWindow *root_window;
} WindowSelectorApplet;

static GdkFilterReturn
filter_func (GdkXEvent            *xevent,
             GdkEvent             *event,
             WindowSelectorApplet *applet);

static void
window_selector_applet_free (WindowSelectorApplet *applet)
{
        if (applet->root_window) {
                gdk_window_remove_filter (applet->root_window,
                                          (GdkFilterFunc) filter_func,
                                          applet);
        }

        g_slice_free (WindowSelectorApplet, applet);
}

/* Retrieves the UTF-8 property @atom from @window */
static char *
get_utf8_property (WindowSelectorApplet *applet,
                   Window                window,
                   Atom                  atom)
{
        GdkDisplay *display;
        Atom type;
        int format, result;
        gulong nitems, bytes_after;
        guchar *val;
        char *ret;

        display = gtk_widget_get_display (GTK_WIDGET (applet->menu_item));

        type = None;
        val = NULL;
        
        gdk_error_trap_push ();
        result = XGetWindowProperty (GDK_DISPLAY_XDISPLAY (display),
                                     window,
                                     atom,
                                     0,
                                     G_MAXLONG,
                                     False,
                                     applet->utf8_string_atom,
                                     &type,
                                     &format,
                                     &nitems,
                                     &bytes_after,
                                     (gpointer) &val);  
        if (gdk_error_trap_pop () || result != Success)
                return NULL;
  
        if (type != applet->utf8_string_atom || format != 8 || nitems == 0) {
                if (val)
                        XFree (val);

                return NULL;
        }

        if (!g_utf8_validate ((char *) val, nitems, NULL)) {
                g_warning ("Invalid UTF-8 in window title");

                XFree (val);

                return NULL;
        }
        
        ret = g_strndup ((char *) val, nitems);
  
        XFree (val);
  
        return ret;
}

/* Retrieves the text property @atom from @window */
static char *
get_text_property (WindowSelectorApplet *applet,
                   Window                window,
                   Atom                  atom)
{
        GdkDisplay *display;
        XTextProperty text;
        char *ret, **list;
        int result, count;

        display = gtk_widget_get_display (GTK_WIDGET (applet->menu_item));

        gdk_error_trap_push ();
        result = XGetTextProperty (GDK_DISPLAY_XDISPLAY (display),
                                   window,
                                   &text,
                                   atom);
        if (gdk_error_trap_pop () || result == 0)
                return NULL;

        count = gdk_text_property_to_utf8_list
                        (gdk_x11_xatom_to_atom (text.encoding),
                         text.format,
                         text.value,
                         text.nitems,
                         &list);
        if (count > 0) {
                int i;

                ret = list[0];

                for (i = 1; i < count; i++)
                        g_free (list[i]);
                g_free (list);
        } else
                ret = NULL;

        if (text.value)
                XFree (text.value);
  
        return ret;
}

/* Retrieves the title for @window */
static char *
window_get_title (WindowSelectorApplet *applet,
                  Window                window)
{
        char *name;
  
        name = get_utf8_property (applet, window, applet->visible_name_atom);
        if (name == NULL)
                name = get_utf8_property (applet, window, applet->name_atom);
        if (name == NULL)
                name = get_text_property (applet, window, XA_WM_NAME);
        if (name == NULL)
                name = g_strdup (_("(untitled)"));

        return name;
}

/* Window menu item activated. Activate the associated window. */
static void
window_menu_item_activate_cb (GtkWidget            *widget,
                              WindowSelectorApplet *applet)
{
        Window window;
        Screen *screen;
        GtkWidget *toplevel;
        XEvent xev;

        window = GPOINTER_TO_UINT
                        (g_object_get_data (G_OBJECT (widget), "window"));
        screen = GDK_SCREEN_XSCREEN (gtk_widget_get_screen (widget));
        toplevel = gtk_widget_get_toplevel (GTK_WIDGET (applet->menu_item));
  
        /* Send _NET_ACTIVE_WINDOW message */
        xev.xclient.type = ClientMessage;
        xev.xclient.serial = 0;
        xev.xclient.send_event = True;
        xev.xclient.display = DisplayOfScreen (screen);
        xev.xclient.window = window;
        xev.xclient.message_type = applet->active_window_atom; 
        xev.xclient.format = 32;
        xev.xclient.data.l[0] = 2;
        xev.xclient.data.l[1] = gtk_get_current_event_time ();
        xev.xclient.data.l[2] = GDK_WINDOW_XWINDOW (toplevel->window);
        xev.xclient.data.l[3] = 0;
        xev.xclient.data.l[4] = 0;

        XSendEvent (DisplayOfScreen (screen),
	            RootWindowOfScreen (screen),
                    False,
	            SubstructureRedirectMask,
	            &xev);        
}

/* Rebuild the selector menu */
static void
rebuild_menu (WindowSelectorApplet *applet)
{
        GList *kids;
        GdkDisplay *display;
        Atom type;
        int format, result, i;
        gulong nitems, bytes_after;
        Window *windows;

        /* Empty menu */
        kids = gtk_container_get_children (GTK_CONTAINER (applet->menu));
        while (kids) {
                gtk_widget_destroy (kids->data);
                kids = g_list_delete_link (kids, kids);
        }

        /* Retrieve list of app windows from root window */
        display = gtk_widget_get_display (GTK_WIDGET (applet->menu_item));

        type = None;

        gdk_error_trap_push ();
        result = XGetWindowProperty (GDK_DISPLAY_XDISPLAY (display),
                                     GDK_WINDOW_XWINDOW (applet->root_window),
                                     applet->stacking_atom,
                                     0,
                                     G_MAXLONG,
                                     False,
                                     XA_WINDOW,
                                     &type,
                                     &format,
                                     &nitems,
                                     &bytes_after,
                                     (gpointer) &windows);
        if (gdk_error_trap_pop () || result != Success)
                return;

        if (type != XA_WINDOW) {
                XFree (windows);

                return;
        }

        /* Load into menu */
        for (i = 0; i < nitems; i++) {
                char *title;
                GtkWidget *menu_item;

                title = window_get_title (applet, windows[i]);
                menu_item = gtk_image_menu_item_new_with_label (title);
                g_free (title);

                g_object_set_data (G_OBJECT (menu_item),
                                   "window",
                                   GUINT_TO_POINTER (windows[i]));

                g_signal_connect (menu_item,
                                  "activate",
                                  G_CALLBACK (window_menu_item_activate_cb),
                                  applet);

                gtk_menu_shell_prepend (GTK_MENU_SHELL (applet->menu),
                                        menu_item);
                gtk_widget_show (menu_item);
        }

        /* If no windows were found, insert an insensitive "No tasks" item */
        if (nitems == 0) {
                GtkWidget *menu_item;
                
                menu_item = gtk_menu_item_new_with_label (_("No tasks"));

                gtk_widget_set_sensitive (menu_item, FALSE);

                gtk_menu_shell_prepend (GTK_MENU_SHELL (applet->menu),
                                        menu_item);
                gtk_widget_show (menu_item);
        }

        /* Cleanup */
        XFree (windows);
}

/* Menu was hidden */
static void
menu_hide_cb (GtkMenuShell         *menu_shell,
              WindowSelectorApplet *applet)
{
        /* Detach menu. This will cause it be destroyed. */
        gtk_menu_item_remove_submenu (applet->menu_item);
}

/* Button press event received on the main menu item */
static gboolean
button_press_event_cb (GtkWidget            *widget,
                       GdkEventButton       *event,
                       WindowSelectorApplet *applet)
{
        GtkWidget *menu;

        /* Set up menu */
        menu = gtk_menu_new ();
        applet->menu = GTK_MENU (menu);

        g_object_add_weak_pointer (G_OBJECT (menu), (gpointer) &applet->menu);

        gtk_menu_item_set_submenu (applet->menu_item, menu);

        g_signal_connect (menu,
                          "hide",
                          G_CALLBACK (menu_hide_cb),
                          applet);
        
        rebuild_menu (applet);

        /* Continue processing. The builtin default handler will eventually
         * cause the menu to pop up. */
        return FALSE;
}

/* Something happened on the root window */
static GdkFilterReturn
filter_func (GdkXEvent            *xevent,
             GdkEvent             *event,
             WindowSelectorApplet *applet)
{
        XEvent *xev;

        xev = (XEvent *) xevent;

        if (xev->type == PropertyNotify) {
                if (xev->xproperty.atom == applet->stacking_atom) {
                        /* _MB_APP_WINDOW_LIST_STACKING changed.
                         * Rebuild menu if around. */
                        if (applet->menu && GTK_WIDGET_VISIBLE (applet->menu))
                                rebuild_menu (applet);
                } else if (xev->xproperty.atom == applet->active_window_atom) {
                        /* _NET_ACTIVE_WINDOW changed.
                         * Update "active task" icon.
                         * TODO */
                }
        }

        return GDK_FILTER_CONTINUE;
}

/* Screen changed */
static void
screen_changed_cb (GtkWidget         *button,
                   GdkScreen         *old_screen,
                   WindowSelectorApplet *applet)
{
        GdkScreen *screen;
        GdkDisplay *display;
        GdkEventMask events;

        if (applet->root_window) {
                gdk_window_remove_filter (applet->root_window,
                                          (GdkFilterFunc) filter_func,
                                          applet);
        }

        screen = gtk_widget_get_screen (button);
        display = gdk_screen_get_display (screen);

        /* Get atoms */
        applet->stacking_atom =
                gdk_x11_get_xatom_by_name_for_display
                        (display, "_MB_APP_WINDOW_LIST_STACKING");
        applet->utf8_string_atom =
                gdk_x11_get_xatom_by_name_for_display
                        (display, "UTF8_STRING");
        applet->name_atom = 
                gdk_x11_get_xatom_by_name_for_display
                        (display, "_NET_WM_NAME");
        applet->visible_name_atom = 
                gdk_x11_get_xatom_by_name_for_display
                        (display, "_NET_WM_VISIBLE_NAME");
        applet->active_window_atom = 
                gdk_x11_get_xatom_by_name_for_display
                        (display, "_NET_ACTIVE_WINDOW");
        
        /* Get root window */
        applet->root_window = gdk_screen_get_root_window (screen);

        /* Watch _MB_APP_WINDOW_LIST_STACKING */
        events = gdk_window_get_events (applet->root_window);
        if ((events & GDK_PROPERTY_CHANGE_MASK) == 0) {
                gdk_window_set_events (applet->root_window,
                                       events & GDK_PROPERTY_CHANGE_MASK);
        }
        
        gdk_window_add_filter (applet->root_window,
                               (GdkFilterFunc) filter_func,
                               applet);
        
        /* Rebuild menu if around */
        if (applet->menu && GTK_WIDGET_VISIBLE (applet->menu))
                rebuild_menu (applet);
}

G_MODULE_EXPORT GtkWidget *
mb_panel_applet_create (const char    *id,
                        GtkOrientation orientation)
{
        WindowSelectorApplet *applet;
        GtkWidget *menu_bar, *menu_item, *image;

        /* Create applet data structure */
        applet = g_slice_new (WindowSelectorApplet);

        applet->root_window = NULL;
        applet->menu = NULL;

        /* Create menu bar */
        menu_bar = gtk_menu_bar_new ();

        gtk_widget_set_name (menu_bar, "MatchboxPanelWindowSelector");

        g_signal_connect (menu_bar,
                          "screen-changed",
                          G_CALLBACK (screen_changed_cb),
                          applet);

        g_object_weak_ref (G_OBJECT (menu_bar),
                           (GWeakNotify) window_selector_applet_free,
                           applet);

        /* Create menu item */
        menu_item = gtk_menu_item_new ();
        applet->menu_item = GTK_MENU_ITEM (menu_item);

        g_signal_connect (menu_item,
                          "button-press-event",
                          G_CALLBACK (button_press_event_cb),
                          applet);

        image = gtk_image_new ();
        applet->image = GTK_IMAGE (image);

        gtk_container_add (GTK_CONTAINER (menu_item), image);

        gtk_menu_shell_append (GTK_MENU_SHELL (menu_bar), menu_item);

        /* Show! */
        gtk_widget_show_all (menu_bar);

        return menu_bar;
};
