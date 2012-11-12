#ifndef __HUD_CLIENT_QUERY_H__
#define __HUD_CLIENT_QUERY_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define HUD_CLIENT_TYPE_QUERY            (hud_client_query_get_type ())
#define HUD_CLIENT_QUERY(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), HUD_CLIENT_TYPE_QUERY, HudClientQuery))
#define HUD_CLIENT_QUERY_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), HUD_CLIENT_TYPE_QUERY, HudClientQueryClass))
#define HUD_CLIENT_IS_QUERY(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HUD_CLIENT_TYPE_QUERY))
#define HUD_CLIENT_IS_QUERY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), HUD_CLIENT_TYPE_QUERY))
#define HUD_CLIENT_QUERY_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), HUD_CLIENT_TYPE_QUERY, HudClientQueryClass))

typedef struct _HudClientQuery         HudClientQuery;
typedef struct _HudClientQueryClass    HudClientQueryClass;
typedef struct _HudClientQueryPrivate  HudClientQueryPrivate;

struct _HudClientQueryClass {
	GObjectClass parent_class;
};

struct _HudClientQuery {
	GObject parent;
	HudClientQueryPrivate * priv;
};

GType hud_client_query_get_type (void);

G_END_DECLS

#endif
