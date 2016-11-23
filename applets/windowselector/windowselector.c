/*
 * (C) 2006, 2007 OpenedHand Ltd.
 *
 * Authors: Jorn Baayen <jorn@openedhand.com>
 *          Ross Burton <ross@openedhand.com>
 *
 * Licensed under the GPL v2 or greater.
 */

#include <config.h>
#include <string.h>
#include <X11/Xatom.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>

#include <matchbox-panel/mb-panel.h>
#include <matchbox-panel/mb-panel-scaling-image2.h>

#define DEFAULT_WINDOW_ICON_NAME "application-x-executable"

enum {
        _MB_APP_WINDOW_LIST_STACKING,
        _MB_CURRENT_APP_WINDOW,
        UTF8_STRING,
        _NET_WM_VISIBLE_NAME,
        _NET_WM_NAME,
        _NET_ACTIVE_WINDOW,
        _NET_WM_ICON,
        N_ATOMS
};

typedef enum {
        MODE_STATIC_ICON,
        MODE_DYNAMIC_ICON,
        MODE_ICON_NAME,
        MODE_NAME
} WindowSelectorAppletMode;

typedef struct {
        GtkWidget *button;
        GtkWidget *menu;
        GtkWidget *image;
        GtkWidget *static_image;
        GtkWidget *label;

        /* If the menu will be showing images. */
        gboolean show_images;

        Atom atoms[N_ATOMS];
        
        GdkWindow *root_window;

        WindowSelectorAppletMode mode;
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

        display = gtk_widget_get_display (GTK_WIDGET (applet->button));

        type = None;
        val = NULL;
        
        gdk_error_trap_push ();
        result = XGetWindowProperty (GDK_DISPLAY_XDISPLAY (display),
                                     window,
                                     atom,
                                     0,
                                     G_MAXLONG,
                                     False,
                                     applet->atoms[UTF8_STRING],
                                     &type,
                                     &format,
                                     &nitems,
                                     &bytes_after,
                                     (gpointer) &val);  
        if (gdk_error_trap_pop () || result != Success)
                return NULL;
  
        if (type != applet->atoms[UTF8_STRING] || format != 8 || nitems == 0) {
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

        display = gtk_widget_get_display (GTK_WIDGET (applet->button));

        gdk_error_trap_push ();
        result = XGetTextProperty (GDK_DISPLAY_XDISPLAY (display),
                                   window,
                                   &text,
                                   atom);
        if (gdk_error_trap_pop () || result == 0)
                return NULL;

        count = gdk_text_property_to_utf8_list_for_display
                        (display,
                         gdk_x11_xatom_to_atom (text.encoding),
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

/* Retrieves the name for @window */
static char *
window_get_name (WindowSelectorApplet *applet,
                 Window                window)
{
        char *name;
  
        name = get_utf8_property (applet,
                                  window,
                                  applet->atoms[_NET_WM_VISIBLE_NAME]);
        if (name == NULL) {
                name = get_utf8_property (applet,
                                          window,
                                          applet->atoms[_NET_WM_NAME]);
        } if (name == NULL) {
                name = get_text_property (applet,
                                          window,
                                          XA_WM_NAME);
        } if (name == NULL) {
                name = g_strdup (_("(untitled)"));
        }

        return name;
}

/* Retrieves the icon for @window */
static GdkPixbuf *
window_get_icon (WindowSelectorApplet *applet,
                 Window                window)
{
        GdkPixbuf *pixbuf;
        GdkDisplay *display;
        Atom type;
        int format, result;
        int ideal_width, ideal_height, ideal_size;
        int best_width, best_height, best_size;
        int i, npixels, ip;
        gulong nitems, bytes_after, *data, *datap, *best_data;
        GtkSettings *settings;
        guchar *pixdata;

        /* First, we read the contents of the _NET_WM_ICON property */
        display = gtk_widget_get_display (GTK_WIDGET (applet->button));

        type = 0;

        gdk_error_trap_push ();
        result = XGetWindowProperty (GDK_DISPLAY_XDISPLAY (display),
                                     window,
                                     applet->atoms[_NET_WM_ICON],
                                     0,
                                     G_MAXLONG,
                                     False,
                                     XA_CARDINAL,
                                     &type,
                                     &format,
                                     &nitems,
                                     &bytes_after,
                                     (gpointer) &data);
        if (gdk_error_trap_pop () || result != Success)
                return NULL;

        if (type != XA_CARDINAL || nitems < 3) {
                XFree (data);

                return NULL;
        }

        /* Got it. Now what size icon are we looking for? */
        settings = gtk_widget_get_settings (GTK_WIDGET (applet->button));
        gtk_icon_size_lookup_for_settings (settings,
                                           GTK_ICON_SIZE_MENU,
                                           &ideal_width,
                                           &ideal_height);

        ideal_size = (ideal_width + ideal_height) / 2;

        /* Try to find the closest match */
        best_data = NULL;
        best_width = best_height = best_size = 0;

        datap = data;
        while (nitems > 0) {
                int cur_width, cur_height, cur_size;
                gboolean replace;

                if (nitems < 3)
                        break;

                cur_width = datap[0];
                cur_height = datap[1];
                cur_size = (cur_width + cur_height) / 2;

                if (nitems < (gulong)(2 + cur_width * cur_height))
                        break;

                if (!best_data) {
                        replace = TRUE;
                } else {
                        /* Always prefer bigger to smaller */
                        if (best_size < ideal_size &&
                            cur_size > best_size)
                                replace = TRUE;
                        /* Prefer smaller bigger */
                        else if (best_size > ideal_size &&
                                 cur_size >= ideal_size && 
                                 cur_size < best_size)
                                replace = TRUE;
                        else
                                replace = FALSE;
                }

                if (replace) {
                        best_data = datap + 2;
                        best_width = cur_width;
                        best_height = cur_height;
                        best_size = cur_size;
                }

                datap += (2 + cur_width * cur_height);
                nitems -= (2 + cur_width * cur_height);
        }

        if (!best_data) {
                XFree (data);

                return NULL;
        }

        /* Got it. Load it into a pixbuf. */
        npixels = best_width * best_height;
        pixdata = g_new (guchar, npixels * 4);
        
        for (i = 0, ip = 0; i < npixels; i++) {
                /* red */
                pixdata[ip] = (best_data[i] >> 16) & 0xff;
                ip++;

                /* green */
                pixdata[ip] = (best_data[i] >> 8) & 0xff;
                ip++;

                /* blue */
                pixdata[ip] = best_data[i] & 0xff;
                ip++;

                /* alpha */
                pixdata[ip] = best_data[i] >> 24;
                ip++;
        }

        pixbuf = gdk_pixbuf_new_from_data (pixdata,
                                           GDK_COLORSPACE_RGB,
                                           TRUE,
                                           8,
                                           best_width,
                                           best_height,
                                           best_width * 4,
                                           (GdkPixbufDestroyNotify) g_free,
                                           NULL);

        /* Scale if necessary */
        if (best_width != ideal_width &&
            best_height != ideal_height) {
                GdkPixbuf *scaled;

                scaled = gdk_pixbuf_scale_simple (pixbuf,
                                                  ideal_width,
                                                  ideal_height,
                                                  GDK_INTERP_BILINEAR);
                g_object_unref (pixbuf);

                pixbuf = scaled;
        }

        /* Cleanup */
        XFree (data);
  
        /* Return */
        return pixbuf;
}

static Window
get_current_app_window (WindowSelectorApplet *applet)
{
        GdkDisplay *display;
        Atom type;
        int format, result;
        gulong nitems, bytes_after, *data;
        Window window;

        display = gtk_widget_get_display (GTK_WIDGET (applet->button));

        type = 0;

        gdk_error_trap_push ();


        result = XGetWindowProperty (GDK_DISPLAY_XDISPLAY (display),
                                     GDK_WINDOW_XID (applet->root_window),
                                     applet->atoms[_MB_CURRENT_APP_WINDOW],
                                     0, 1,
                                     False,
                                     AnyPropertyType,
                                     &type,
                                     &format,
                                     &nitems,
                                     &bytes_after,
                                     (gpointer) &data);


        if (gdk_error_trap_pop () || result != Success)
                return 0;

        if (type != XA_WINDOW || nitems < 1) {
                XFree (data);

                return 0;
        }

        window = (Window)data[0];
        XFree (data);

        return window;
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
        toplevel = gtk_widget_get_toplevel (GTK_WIDGET (applet->button));

        /* Send _NET_ACTIVE_WINDOW message */
        xev.xclient.type = ClientMessage;
        xev.xclient.serial = 0;
        xev.xclient.send_event = True;
        xev.xclient.display = DisplayOfScreen (screen);
        xev.xclient.window = window;
        xev.xclient.message_type = applet->atoms[_NET_ACTIVE_WINDOW];
        xev.xclient.format = 32;
        xev.xclient.data.l[0] = 2;
        xev.xclient.data.l[1] = gtk_get_current_event_time ();
        xev.xclient.data.l[2] = GDK_WINDOW_XID (gtk_widget_get_window (toplevel));
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
        int format, result;
        gulong i;
        gulong nitems, bytes_after;
        Window *windows;

        /* Empty menu */
        kids = gtk_container_get_children (GTK_CONTAINER (applet->menu));
        while (kids) {
                gtk_widget_destroy (kids->data);
                kids = g_list_delete_link (kids, kids);
        }

        /* Retrieve list of app windows from root window */
        display = gtk_widget_get_display (GTK_WIDGET (applet->button));

        type = None;

        gdk_error_trap_push ();
        result = XGetWindowProperty (GDK_DISPLAY_XDISPLAY (display),
                                     GDK_WINDOW_XID (applet->root_window),
                                     applet->atoms
                                             [_MB_APP_WINDOW_LIST_STACKING],
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
                char *name;
                GtkWidget *menu_item;

                name = window_get_name (applet, windows[i]);
                menu_item = gtk_image_menu_item_new_with_label (name);
                g_free (name);

                if (applet->show_images) {
                        GtkWidget *image;
                        GdkPixbuf *icon;
                        
                        image = gtk_image_new ();

                        icon = window_get_icon (applet, windows[i]);
                        if (icon) {
                                gtk_image_set_from_pixbuf (GTK_IMAGE (image),
                                                           icon);
                                g_object_unref (icon);
                        } else {
                                gtk_image_set_from_icon_name
                                        (GTK_IMAGE (image),
                                         DEFAULT_WINDOW_ICON_NAME,
                                         GTK_ICON_SIZE_MENU);
                        }
                        
                        gtk_image_menu_item_set_image
                                (GTK_IMAGE_MENU_ITEM (menu_item), image);
                        gtk_widget_show (image);
                }

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

/* Menu was deactivated */
static void
menu_selection_done_cb (GtkMenuShell         *menu_shell,
                        WindowSelectorApplet *applet)
{
        gtk_widget_destroy (GTK_WIDGET (menu_shell));

        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (applet->button),
                                      FALSE);
}

static void
position_menu (GtkMenu  *menu,
               int      *x,
               int      *y,
               gboolean *push_in,
               gpointer  user_data)
{
        WindowSelectorApplet *applet = user_data;
        GtkAllocation allocation;

        gdk_window_get_origin (gtk_widget_get_window (applet->button), x, y);
        gtk_widget_get_allocation (applet->button, &allocation);

        *x += allocation.x;
        *y += allocation.height;
        *push_in = TRUE;
}

static void
toggled_cb (GtkToggleButton      *button,
            WindowSelectorApplet *applet)
{
        GtkWidget *menu;
        
        if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (applet->button)))
                return;
        
        /* Set up menu */
        menu = gtk_menu_new ();
        applet->menu = menu;

        g_object_add_weak_pointer (G_OBJECT (menu), (gpointer) &applet->menu);

        g_signal_connect (menu,
                          "selection-done",
                          G_CALLBACK (menu_selection_done_cb),
                          applet);
        
        rebuild_menu (applet);

        gtk_menu_popup (GTK_MENU (menu), NULL, NULL,
                        position_menu, applet,
                        0, gtk_get_current_event_time ());
}

static void
update_current_app (WindowSelectorApplet *applet)
{
        Window window;

        if (applet->mode == MODE_STATIC_ICON)
                return;

        window = get_current_app_window (applet);

        if (window)
        {
                if (applet->mode == MODE_DYNAMIC_ICON 
                                || applet->mode == MODE_ICON_NAME)
                {
                        gtk_image_set_from_pixbuf (GTK_IMAGE (applet->image), 
                                        window_get_icon (applet, window));
                        gtk_widget_show (applet->image);
                }

                if (applet->mode == MODE_ICON_NAME
                                || applet->mode == MODE_NAME)
                {
                        gchar *name = NULL;
                        name = window_get_name (applet, window);
                        gtk_label_set_text (GTK_LABEL (applet->label), 
                                        name);
                        g_free (name);
                }
        } else {
                if (applet->mode == MODE_DYNAMIC_ICON
                                || applet->mode == MODE_ICON_NAME)
                {
                        gtk_widget_hide (applet->image);
                }

                if (applet->mode == MODE_ICON_NAME
                                || applet->mode == MODE_NAME)
                {
                        gtk_label_set_text (GTK_LABEL (applet->label),
                                        _("No tasks"));
                }
        }
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
                if (xev->xproperty.atom ==
                    applet->atoms[_MB_APP_WINDOW_LIST_STACKING]) {
                        /* _MB_APP_WINDOW_LIST_STACKING changed.
                         * Rebuild menu if around. */
                        if (applet->menu && gtk_widget_get_visible (applet->menu))
                                rebuild_menu (applet);
                }
                if (xev->xproperty.atom ==
                    applet->atoms[_MB_CURRENT_APP_WINDOW]) {
                        update_current_app (applet);
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

        /* VILE

           The property GtkSetting::gtk-image-menus doesn't exist until a
           GtkImageMenuItem has been created, and that happens after the
           initial screen-changed signal.

           The clean alternative is to check the value of the property every
           time the menu is created, which is probably only fractionally
           slower.
        */
        gtk_widget_destroy (gtk_image_menu_item_new ());

        /* Get settings */
        g_object_get (gtk_settings_get_for_screen (screen),
                      "gtk-menu-images", &applet->show_images,
                      NULL);

        /* Get atoms */
        applet->atoms[_MB_APP_WINDOW_LIST_STACKING] =
                gdk_x11_get_xatom_by_name_for_display
                        (display, "_MB_APP_WINDOW_LIST_STACKING");
        applet->atoms[_MB_CURRENT_APP_WINDOW] =
                gdk_x11_get_xatom_by_name_for_display
                        (display, "_MB_CURRENT_APP_WINDOW");
        applet->atoms[UTF8_STRING] =
                gdk_x11_get_xatom_by_name_for_display
                        (display, "UTF8_STRING");
        applet->atoms[_NET_WM_NAME] =
                gdk_x11_get_xatom_by_name_for_display
                        (display, "_NET_WM_NAME");
        applet->atoms[_NET_WM_VISIBLE_NAME] =
                gdk_x11_get_xatom_by_name_for_display
                        (display, "_NET_WM_VISIBLE_NAME");
        applet->atoms[_NET_WM_ICON] =
                gdk_x11_get_xatom_by_name_for_display
                        (display, "_NET_WM_ICON");
        applet->atoms[_NET_ACTIVE_WINDOW] =
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
        if (applet->menu && gtk_widget_get_visible (applet->menu))
                rebuild_menu (applet);

        update_current_app (applet);
}

G_MODULE_EXPORT GtkWidget *
mb_panel_applet_create (const char    *id,
                        GtkOrientation orientation)
{
        WindowSelectorApplet *applet;
        GtkWidget *hbox;

        /* Create applet data structure */
        applet = g_slice_new0 (WindowSelectorApplet);

        /* identify the mode */
        if (id == NULL || strlen (id) == 0)
        {
                applet->mode = MODE_STATIC_ICON;
        } else if (strcmp (id, "static-icon") == 0)
        {
                applet->mode = MODE_STATIC_ICON;
        } else if (strcmp (id, "dynamic-icon") == 0)
        {
                applet->mode = MODE_DYNAMIC_ICON;
        } else if (strcmp (id, "icon-name") == 0)
        {
                applet->mode = MODE_ICON_NAME;
        } else if (strcmp (id, "name") == 0)
        {
                applet->mode = MODE_NAME;
        } else  {
                g_warning ("Unknown mode given as id");
                applet->mode = MODE_STATIC_ICON;
        }

        /* The button itself */
        applet->button = gtk_toggle_button_new ();
        gtk_button_set_relief (GTK_BUTTON (applet->button), GTK_RELIEF_NONE);
        gtk_widget_set_name (applet->button, "MatchboxPanelWindowSelector");


        switch (applet->mode) {
        case MODE_STATIC_ICON:
                applet->image = mb_panel_scaling_image2_new (orientation,
                        "panel-task-switcher");
                gtk_container_add (GTK_CONTAINER (applet->button),
                                applet->image);
                break;
        case MODE_DYNAMIC_ICON:
                applet->image = gtk_image_new ();
                gtk_container_add (GTK_CONTAINER (applet->button),
                                applet->image);
                break;
        case MODE_ICON_NAME:
                hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
                applet->label = gtk_label_new (NULL);
                applet->image = gtk_image_new ();
                gtk_box_pack_start (GTK_BOX (hbox), applet->image,
                                FALSE, FALSE, 2);
                gtk_box_pack_start (GTK_BOX (hbox), applet->label,
                                TRUE, TRUE, 0);
                gtk_container_add (GTK_CONTAINER (applet->button), hbox);
                break;
        case MODE_NAME:
                applet->label = gtk_label_new (NULL);
                gtk_container_add (GTK_CONTAINER (applet->button), applet->label);
                break;
        }

        /* TODO: also pack an arrow? */

        g_signal_connect (applet->button,
                          "screen-changed",
                          G_CALLBACK (screen_changed_cb),
                          applet);

        g_signal_connect (applet->button,
                          "toggled",
                          G_CALLBACK (toggled_cb),
                          applet);

        g_object_weak_ref (G_OBJECT (applet->button),
                           (GWeakNotify) window_selector_applet_free,
                           applet);

        /* Show! */
        gtk_widget_show_all (applet->button);

        return applet->button;
};
