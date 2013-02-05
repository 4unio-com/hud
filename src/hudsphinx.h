#ifndef HUDSPHINX_H_
#define HUDSPHINX_H_

#include <glib-object.h>

/*
 * Type macros.
 */
#define HUD_TYPE_SPHINX                  (hud_sphinx_get_type ())
#define HUD_SPHINX(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), HUD_TYPE_SPHINX, HudSphinx))
#define HUD_IS_SPHINX(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HUD_TYPE_SPHINX))
#define HUD_SPHINX_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), HUD_TYPE_SPHINX, HudSphinxClass))
#define HUD_IS_SPHINX_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), HUD_TYPE_SPHINX))
#define HUD_SPHINX_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), HUD_TYPE_SPHINX, HudSphinxClass))

typedef struct _HudSphinx        HudSphinx;

typedef struct _HudQueryIfaceComCanonicalHudQuery HudQueryIfaceComCanonicalHudQuery;
typedef struct _HudSource         HudSource;

/* used by HUD_TYPE_SPHINX */
GType hud_sphinx_get_type (void);

/*
 * Method definitions.
 */

HudSphinx * hud_sphinx_new (HudQueryIfaceComCanonicalHudQuery * skel);

gchar * hud_sphinx_voice_query (HudSphinx *self, HudSource *source);

#endif /* HUDSPHINX_H_ */
