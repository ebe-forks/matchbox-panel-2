#ifndef _MB_NOTIFY_STORE
#define _MB_NOTIFY_STORE

#include <glib-object.h>

G_BEGIN_DECLS

#define MB_TYPE_NOTIFY_STORE mb_notify_store_get_type()

#define MB_NOTIFY_STORE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MB_TYPE_NOTIFY_STORE, MbNotifyStore))

#define MB_NOTIFY_STORE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MB_TYPE_NOTIFY_STORE, MbNotifyStoreClass))

#define MB_IS_NOTIFY(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MB_TYPE_NOTIFY_STORE))

#define MB_IS_NOTIFY_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MB_TYPE_NOTIFY_STORE))

#define MB_NOTIFY_STORE_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MB_TYPE_NOTIFY_STORE, MbNotifyStoreClass))

typedef struct {
  guint id;
  char *summary;
  char *body;
  char *icon_name;
  guint timeout_id;
} Notification;

typedef struct {
  GObject parent;
} MbNotifyStore;

typedef struct {
  GObjectClass parent_class;
  
  void *(*notification_added) (MbNotifyStore *notify, Notification *notification);
  void *(*notification_closed) (MbNotifyStore *notify, guint id, guint reason);
} MbNotifyStoreClass;

GType mb_notify_store_get_type (void);

MbNotifyStore* mb_notify_store_new (void);

void mb_notify_store_close (MbNotifyStore *notify, guint id /* TODO: reason */);

G_END_DECLS

#endif /* _MB_NOTIFY_STORE */
