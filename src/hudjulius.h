#ifndef HUDJULIUS_H_
#define HUDJULIUS_H_

#include <glib-object.h>

/*
 * Type macros.
 */
#define HUD_TYPE_JULIUS                  (hud_julius_get_type ())
#define HUD_JULIUS(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), HUD_TYPE_JULIUS, HudJulius))
#define HUD_IS_JULIUS(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HUD_TYPE_JULIUS))
#define HUD_JULIUS_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), HUD_TYPE_JULIUS, HudJuliusClass))
#define HUD_IS_JULIUS_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), HUD_TYPE_JULIUS))
#define HUD_JULIUS_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), HUD_TYPE_JULIUS, HudJuliusClass))

typedef struct _HudJulius        HudJulius;
typedef struct _HudJuliusClass   HudJuliusClass;

struct _HudJulius
{
  GObject parent_instance;

  /* instance members */
};

struct _HudJuliusClass
{
  GObjectClass parent_class;

  /* class members */
};

/* used by HUD_TYPE_JULIUS */
GType hud_julius_get_type (void);

/*
 * Method definitions.
 */

#endif /* HUDJULIUS_H_ */
