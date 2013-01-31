#ifndef __HUD_CLIENT_PARAM_H__
#define __HUD_CLIENT_PARAM_H__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define HUD_CLIENT_TYPE_PARAM            (hud_client_param_get_type ())
#define HUD_CLIENT_PARAM(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), HUD_CLIENT_TYPE_PARAM, HudClientParam))
#define HUD_CLIENT_PARAM_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), HUD_CLIENT_TYPE_PARAM, HudClientParamClass))
#define HUD_CLIENT_IS_PARAM(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HUD_CLIENT_TYPE_PARAM))
#define HUD_CLIENT_IS_PARAM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), HUD_CLIENT_TYPE_PARAM))
#define HUD_CLIENT_PARAM_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), HUD_CLIENT_TYPE_PARAM, HudClientParamClass))

typedef struct _HudClientParam          HudClientParam;
typedef struct _HudClientParamClass     HudClientParamClass;
typedef struct _HudClientParamPrivate   HudClientParamPrivate;

struct _HudClientParamClass {
	GObjectClass parent_class;
};

struct _HudClientParam {
	GObject parent;
	HudClientParamPrivate * priv;
};

GType hud_client_param_get_type (void);

G_END_DECLS

#endif
