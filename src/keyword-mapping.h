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
 *
 * Authors: Pete Woods <pete.woods@canonical.com>
 */

#ifndef __HUDKEYWORDMAPPING_H__
#define __HUDKEYWORDMAPPING_H__

#include <glib-object.h>
#include <glib.h>

/*
 * Type macros.
 */
#define HUD_TYPE_KEYWORD_MAPPING                  (hud_keyword_mapping_get_type ())
#define HUD_KEYWORD_MAPPING(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), HUD_TYPE_KEYWORD_MAPPING, HudKeywordMapping))
#define HUD_IS_KEYWORD_MAPPING(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HUD_TYPE_KEYWORD_MAPPING))
#define HUD_KEYWORD_MAPPING_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), HUD_TYPE_KEYWORD_MAPPING, HudKeywordMappingClass))
#define HUD_IS_KEYWORD_MAPPING_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), HUD_TYPE_KEYWORD_MAPPING))
#define HUD_KEYWORD_MAPPING_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), HUD_TYPE_KEYWORD_MAPPING, HudKeywordMappingClass))

typedef struct _HudKeywordMapping HudKeywordMapping;
//typedef struct _HudKeywordMappingClass HudKeywordMappingClass;

/* used by HUD_TYPE_KEYWORD_MAPPING */
GType hud_keyword_mapping_get_type(void);

/*
 * Method definitions.
 */

HudKeywordMapping *hud_keyword_mapping_new(void);

void hud_keyword_mapping_load (HudKeywordMapping *self,
    const gchar *app_id, const gchar *datadir, const gchar *localedir);

GPtrArray* hud_keyword_mapping_transform(HudKeywordMapping *self, const gchar* label);

#endif /* __HUDKEYWORDMAPPING_H__ */
