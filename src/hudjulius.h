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

#ifndef HUDJULIUS_H_
#define HUDJULIUS_H_

#include <hudvoice.h>

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

typedef struct _HudQueryIfaceComCanonicalHudQuery HudQueryIfaceComCanonicalHudQuery;

/* Default timeout in microseconds */
#define HUD_JULIUS_DEFAULT_TIMEOUT 10000000

/* used by HUD_TYPE_JULIUS */
GType hud_julius_get_type (void);

/*
 * Method definitions.
 */

gboolean hud_julius_is_installed();

HudJulius * hud_julius_new(HudQueryIfaceComCanonicalHudQuery * skel);

#endif /* HUDJULIUS_H_ */
