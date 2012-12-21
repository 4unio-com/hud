#ifndef __HUD_APPLICATION_SOURCE_H__
#define __HUD_APPLICATION_SOURCE_H__

#include <glib-object.h>
#include <libbamf/libbamf.h>

G_BEGIN_DECLS

#define HUD_TYPE_APPLICATION_SOURCE            (hud_application_source_get_type ())
#define HUD_APPLICATION_SOURCE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), HUD_TYPE_APPLICATION_SOURCE, HudApplicationSource))
#define HUD_APPLICATION_SOURCE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), HUD_TYPE_APPLICATION_SOURCE, HudApplicationSourceClass))
#define HUD_IS_APPLICATION_SOURCE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HUD_TYPE_APPLICATION_SOURCE))
#define HUD_IS_APPLICATION_SOURCE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), HUD_TYPE_APPLICATION_SOURCE))
#define HUD_APPLICATION_SOURCE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), HUD_TYPE_APPLICATION_SOURCE, HudApplicationSourceClass))

typedef struct _HudApplicationSource         HudApplicationSource;
typedef struct _HudApplicationSourceClass    HudApplicationSourceClass;
typedef struct _HudApplicationSourcePrivate  HudApplicationSourcePrivate;

struct _HudApplicationSourceClass {
	GObjectClass parent_class;
};

struct _HudApplicationSource {
	GObject parent;
	HudApplicationSourcePrivate * priv;
};

GType                    hud_application_source_get_type          (void);
HudApplicationSource *   hud_application_source_new_for_app       (BamfApplication * bapp);
gboolean                 hud_application_source_is_empty          (HudApplicationSource * app);


/* Helper functions */
gchar *                  hud_application_source_bamf_app_id       (BamfApplication * bapp);

G_END_DECLS

#endif
