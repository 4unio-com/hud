#if !defined (_HUD_CLIENT_H_INSIDE) && !defined (HUD_CLIENT_COMPILATION)
#error "Only <hud-client.h> can be included directly."
#endif

#ifndef __HUD_CLIENT_CONNECTION_H__
#define __HUD_CLIENT_CONNECTION_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define HUD_CLIENT_TYPE_CONNECTION            (hud_client_connection_get_type ())
#define HUD_CLIENT_CONNECTION(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), HUD_CLIENT_TYPE_CONNECTION, HudClientConnection))
#define HUD_CLIENT_CONNECTION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), HUD_CLIENT_TYPE_CONNECTION, HudClientConnectionClass))
#define HUD_CLIENT_IS_CONNECTION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HUD_CLIENT_TYPE_CONNECTION))
#define HUD_CLIENT_IS_CONNECTION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), HUD_CLIENT_TYPE_CONNECTION))
#define HUD_CLIENT_CONNECTION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), HUD_CLIENT_TYPE_CONNECTION, HudClientConnectionClass))

typedef struct _HudClientConnection         HudClientConnection;
typedef struct _HudClientConnectionClass    HudClientConnectionClass;
typedef struct _HudClientConnectionPrivate  HudClientConnectionPrivate ;

struct _HudClientConnectionClass {
	GObjectClass parent_class;
};

struct _HudClientConnection {
	GObject parent;
	HudClientConnectionPrivate * priv;
};

GType                   hud_client_connection_get_type   (void);
HudClientConnection *   hud_client_connection_get_ref    (void);
HudClientConnection *   hud_client_connection_new        (gchar * dbus_address,
                                                          gchar * dbus_path);

G_END_DECLS

#endif
