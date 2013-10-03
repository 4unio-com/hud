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
 */

#ifndef __HUD_VOICE_H__
#define __HUD_VOICE_H__

#include <glib.h>
#include <glib-object.h>

#define HUD_TYPE_VOICE                                     (hud_voice_get_type ())
#define HUD_VOICE(inst)                                    (G_TYPE_CHECK_INSTANCE_CAST ((inst),                     \
                                                             HUD_TYPE_VOICE, HudVoice))
#define HUD_IS_VOICE(inst)                                 (G_TYPE_CHECK_INSTANCE_TYPE ((inst),                     \
                                                             HUD_TYPE_VOICE))
#define HUD_VOICE_GET_IFACE(inst)                          (G_TYPE_INSTANCE_GET_INTERFACE ((inst),                  \
                                                             HUD_TYPE_VOICE, HudVoiceInterface))

typedef struct _HudVoiceInterface                          HudVoiceInterface;
typedef struct _HudVoice                                   HudVoice;
typedef struct _HudSource                                  HudSource;
typedef struct _HudQueryIfaceComCanonicalHudQuery          HudQueryIfaceComCanonicalHudQuery;

enum HUD_VOICE_ERROR_CODE
{
  HUD_VOICE_HUD_STATE_ERROR,
  HUD_VOICE_INITIALISATION_ERROR,
  HUD_VOICE_AUDIO_DEVICE_OPEN_ERROR,
  HUD_VOICE_READ_ERROR,
  HUD_VOICE_NO_AUDIO_ERROR
};

struct _HudVoiceInterface
{
  GTypeInterface g_iface;

  gboolean (*query) (HudVoice *self, HudSource *source, gchar **result,
      GError **error);
};

GType hud_voice_get_type (void);

HudVoice * hud_voice_new (HudQueryIfaceComCanonicalHudQuery *skel,
    const gchar *device, GError **error);

gboolean hud_voice_query (HudVoice *self, HudSource *source, gchar **result,
    GError **error);

#endif /* __HUD_VOICE_H__ */
