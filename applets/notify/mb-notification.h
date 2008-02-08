#ifndef _MB_NOTIFICATION
#define _MB_NOTIFICATION

#include <gtk/gtkeventbox.h>
#include "notify-store.h"

G_BEGIN_DECLS

#define MB_TYPE_NOTIFICATION mb_notification_get_type()

#define MB_NOTIFICATION(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MB_TYPE_NOTIFICATION, MbNotification))

#define MB_NOTIFICATION_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MB_TYPE_NOTIFICATION, MbNotificationClass))

#define MB_IS_NOTIFICATION(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MB_TYPE_NOTIFICATION))

#define MB_IS_NOTIFICATION_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MB_TYPE_NOTIFICATION))

#define MB_NOTIFICATION_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MB_TYPE_NOTIFICATION, MbNotificationClass))

typedef struct {
  GtkEventBox parent;
} MbNotification;

typedef struct {
  GtkEventBoxClass parent_class;
  void (*closed) (MbNotification *notification);
} MbNotificationClass;

GType mb_notification_get_type (void);

GtkWidget * mb_notification_new (void);

void mb_notification_update (MbNotification *notification, Notification *n);

guint mb_notification_get_id (MbNotification *notification);

G_END_DECLS

#endif /* _MB_NOTIFICATION */
