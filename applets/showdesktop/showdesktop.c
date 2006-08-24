/* 
 * (C) 2006 OpenedHand Ltd.
 *
 * Author: Jorn Baayen <jorn@openedhand.com>
 *
 * Licensed under the GPL v2 or greater.
 */

#include <gtk/gtktogglebutton.h>
#include <gdk/gdkx.h>
#include <X11/Xatom.h>
#include <matchbox-panel/mb-panel.h>
#include <matchbox-panel/mb-panel-scaling-image.h>

typedef struct {
        GtkToggleButton *button;

        gboolean block_toggle;

        Atom atom;
        
        GdkWindow *root_window;
} ShowDesktopApplet;

static GdkFilterReturn
filter_func (GdkXEvent         *xevent,
             GdkEvent          *event,
             ShowDesktopApplet *applet);

static void
show_desktop_applet_free (ShowDesktopApplet *applet)
{
        if (applet->root_window) {
                gdk_window_remove_filter (applet->root_window,
                                          (GdkFilterFunc) filter_func,
                                          applet);
        }

        g_slice_free (ShowDesktopApplet, applet);
}

/* Sync @applet with the _NET_SHOWING_DESKTOP root window property */
static void
sync (ShowDesktopApplet *applet)
{
        GdkDisplay *display;
        Atom type;
        int format, result;
        gulong nitems, bytes_after, *num;

        display = gtk_widget_get_display (GTK_WIDGET (applet->button));

        type = 0;

        gdk_error_trap_push ();
        result = XGetWindowProperty (GDK_DISPLAY_XDISPLAY (display),
                                     GDK_WINDOW_XWINDOW
                                            (applet->root_window),
                                     applet->atom,
                                     0,
                                     G_MAXLONG,
                                     False,
                                     XA_CARDINAL,
                                     &type,
                                     &format,
                                     &nitems,
                                     &bytes_after,
                                     (gpointer) &num);
        if (!gdk_error_trap_pop () && result == Success) {
                if (type == XA_CARDINAL && nitems > 0) {
                        applet->block_toggle = TRUE;

                        gtk_toggle_button_set_active (applet->button, *num);

                        applet->block_toggle = FALSE;
                }

                XFree (num);
        }
}

/* Something happened on the root window */
static GdkFilterReturn
filter_func (GdkXEvent         *xevent,
             GdkEvent          *event,
             ShowDesktopApplet *applet)
{
        XEvent *xev;

        xev = (XEvent *) xevent;

        if (xev->type == PropertyNotify) {
                if (xev->xproperty.atom == applet->atom) {
                        /* _NET_SHOWING_DESKTOP changed */
                        sync (applet);
                }
        }

        return GDK_FILTER_CONTINUE;
}

/* Screen changed */
static void
screen_changed_cb (GtkWidget         *button,
                   GdkScreen         *old_screen,
                   ShowDesktopApplet *applet)
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

        applet->atom = gdk_x11_get_xatom_by_name_for_display
                                (display, "_NET_SHOWING_DESKTOP");
        applet->root_window = gdk_screen_get_root_window (screen);

        /* Watch _NET_SHOWING_DESKTOP */
        events = gdk_window_get_events (applet->root_window);
        if ((events & GDK_PROPERTY_CHANGE_MASK) == 0) {
                gdk_window_set_events (applet->root_window,
                                       events & GDK_PROPERTY_CHANGE_MASK);
        }

        gdk_window_add_filter (applet->root_window,
                               (GdkFilterFunc) filter_func,
                               applet);

        /* Sync */
        sync (applet);
}

/* Button toggled */
static void
button_toggled_cb (GtkToggleButton   *button,
                   ShowDesktopApplet *applet)
{
        GtkWidget *widget;
        Screen *screen;
        XEvent xev;

        if (applet->block_toggle)
                return;

        widget = GTK_WIDGET (button);
        screen = GDK_SCREEN_XSCREEN (gtk_widget_get_screen (widget));
  
        xev.xclient.type = ClientMessage;
        xev.xclient.serial = 0;
        xev.xclient.send_event = True;
        xev.xclient.display = DisplayOfScreen (screen);
        xev.xclient.window = RootWindowOfScreen (screen);
        xev.xclient.message_type = applet->atom; 
        xev.xclient.format = 32;
        xev.xclient.data.l[0] = button->active;
        xev.xclient.data.l[1] = 0;
        xev.xclient.data.l[2] = 0;
        xev.xclient.data.l[3] = 0;
        xev.xclient.data.l[4] = 0;

        XSendEvent (DisplayOfScreen (screen),
	            RootWindowOfScreen (screen),
                    False,
	            SubstructureRedirectMask | SubstructureNotifyMask,
	            &xev);
}

G_MODULE_EXPORT GtkWidget *
mb_panel_applet_create (const char    *id,
                        GtkOrientation orientation)
{
        ShowDesktopApplet *applet;
        GtkWidget *button, *image;

        /* Create applet data structure */
        applet = g_slice_new (ShowDesktopApplet);

        applet->block_toggle = FALSE;
        applet->root_window = NULL;

        /* Create button */
        button = gtk_toggle_button_new ();
        applet->button = GTK_TOGGLE_BUTTON (button);

        GTK_WIDGET_UNSET_FLAGS (button, GTK_CAN_FOCUS);

        gtk_widget_set_name (button, "MatchboxPanelShowDesktop");

        image = mb_panel_scaling_image_new ("gnome-fs-desktop");
        gtk_container_add (GTK_CONTAINER (button), image);

        g_signal_connect (button,
                          "screen-changed",
                          G_CALLBACK (screen_changed_cb),
                          applet);
        g_signal_connect (button,
                          "toggled",
                          G_CALLBACK (button_toggled_cb),
                          applet);

        g_object_weak_ref (G_OBJECT (button),
                           (GWeakNotify) show_desktop_applet_free,
                           applet);

        /* Show! */
        gtk_widget_show_all (button);

        return button;
};
