#include <gtk/gtk.h>
#include "mb-notification.h"

G_DEFINE_TYPE (MbNotification, mb_notification, GTK_TYPE_HBOX)

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MB_TYPE_NOTIFICATION, MbNotificationPrivate))

typedef struct _MbNotificationPrivate {
  guint id;
  GtkWidget *image;
  GtkWidget *label;
} MbNotificationPrivate;

static void
mb_notification_class_init (MbNotificationClass *klass)
{
  g_type_class_add_private (klass, sizeof (MbNotificationPrivate));
}

static void
mb_notification_init (MbNotification *self)
{
  MbNotificationPrivate *priv = GET_PRIVATE (self);
  
  g_object_set (self,
                "border-width", 8,
                "spacing", 8,
                "homogeneous", FALSE,
                NULL);
  
  priv->image = gtk_image_new ();
  gtk_box_pack_start (GTK_BOX (self), priv->image, FALSE, FALSE, 0);
  
  priv->label = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (priv->label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (self), priv->label, TRUE, TRUE, 0);
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
