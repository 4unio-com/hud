/*
 * Copyright © 2012 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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

#endif /* HUDSPHINX_H_ */
