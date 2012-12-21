#ifndef HUDRANDOMSOURCE_H_
#define HUDRANDOMSOURCE_H_

#include <glib-object.h>

/*
 * Type macros.
 */
#define HUD_TYPE_RANDOM_SOURCE                  (hud_random_source_get_type ())
#define HUD_RANDOM_SOURCE(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), HUD_TYPE_RANDOM_SOURCE, HudRandomSource))
#define HUD_IS_RANDOM_SOURCE(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HUD_TYPE_RANDOM_SOURCE))
#define HUD_RANDOM_SOURCE_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), HUD_TYPE_RANDOM_SOURCE, HudRandomSourceClass))
#define HUD_IS_RANDOM_SOURCE_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), HUD_TYPE_RANDOM_SOURCE))
#define HUD_RANDOM_SOURCE_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), HUD_TYPE_RANDOM_SOURCE, HudRandomSourceClass))

typedef struct _HudRandomSource        HudRandomSource;

/* used by HUD_TYPE_RANDOM_SOURCE */
GType hud_random_source_get_type (void);

/*
 * Method definitions.
 */

gchar * hud_random_source_make_words (GRand *rand,
    gint n_words);

HudSource * hud_random_source_new (GRand *rand);

HudSource * hud_random_source_new_full (GRand *rand, const gint max_depth,
    const gint max_items, const gint max_words, const gint max_letters);

#endif /* HUDRANDOMSOURCE_H_ */
