#include <gtk/gtk.h>
#include "mb-notification.h"

G_DEFINE_TYPE (MbNotification, mb_notification, GTK_TYPE_EVENT_BOX);

enum {
  CLOSED,
  N_SIGNALS,
};

static guint signals[N_SIGNALS];

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MB_TYPE_NOTIFICATION, MbNotificationPrivate))

typedef struct _MbNotificationPrivate {
  guint id;
  GtkWidget *image;
  GtkWidget *label;
} MbNotificationPrivate;


static gboolean
on_button_release (MbNotification *notification, GdkEventButton *event)
{
  if (event->button == 1) {
    g_signal_emit (notification, signals[CLOSED], 0);
    return TRUE;
  } else {
    return FALSE;
  }
}

static gboolean
on_draw (GtkWidget *widget, cairo_t *cr)
{
  GtkStyleContext *style;
  int width, height;

  style = gtk_widget_get_style_context (widget);

  width = gtk_widget_get_allocated_width (widget);
  height = gtk_widget_get_allocated_height (widget);

  gtk_render_frame (style, cr, 0, 0, width, height);

  (*GTK_WIDGET_CLASS (mb_notification_parent_class)->draw) (widget, cr);

  return FALSE;
}

static void
mb_notification_class_init (MbNotificationClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MbNotificationPrivate));

  widget_class->draw = on_draw;

  signals[CLOSED] = g_signal_new ("closed",
                                  G_OBJECT_CLASS_TYPE (klass),
                                  G_SIGNAL_RUN_FIRST,
                                  G_STRUCT_OFFSET (MbNotificationClass, closed),
                                  NULL, NULL,
                                  g_cclosure_marshal_VOID__VOID,
                                  G_TYPE_NONE, 0);
}

static void
mb_notification_init (MbNotification *self)
{
  MbNotificationPrivate *priv = GET_PRIVATE (self);
  GtkWidget *box;

  gtk_container_set_border_width (GTK_CONTAINER (self), 8);
  gtk_event_box_set_visible_window (GTK_EVENT_BOX (self), FALSE);
  gtk_widget_add_events (GTK_WIDGET (self), GDK_BUTTON_RELEASE_MASK);
  g_signal_connect (self, "button-release-event", G_CALLBACK (on_button_release), NULL);

  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 8);
  gtk_container_add (GTK_CONTAINER (self), box);

  priv->image = gtk_image_new ();
  gtk_box_pack_start (GTK_BOX (box), priv->image, FALSE, FALSE, 0);

  priv->label = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (priv->label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (box), priv->label, TRUE, TRUE, 0);
}

GtkWidget *
mb_notification_new (void)
{
  return g_object_new (MB_TYPE_NOTIFICATION, NULL);
}

void
mb_notification_update (MbNotification *notification, Notification *n)
{
  MbNotificationPrivate *priv = GET_PRIVATE (notification);
  char *s;

  priv->id = n->id;

  if (n->icon_name) {
    gtk_image_set_from_icon_name (GTK_IMAGE (priv->image),
                                  n->icon_name, GTK_ICON_SIZE_DIALOG);
  } else {
    /* TODO: should this default to no image, or "info" */
    gtk_image_clear (GTK_IMAGE (priv->image));
  }

  s = g_strdup_printf ("<big><b>%s</b></big>\n"
                       "\n%s", n->summary, n->body ?: NULL);
  gtk_label_set_markup (GTK_LABEL (priv->label), s);
  g_free (s);
}

guint
mb_notification_get_id (MbNotification *notification)
{
  MbNotificationPrivate *priv = GET_PRIVATE (notification);
  return priv->id;
}
