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

#ifndef __HUD_UNITY_VOICE_H__
#define __HUD_UNITY_VOICE_H__

#include <glib-object.h>

/*
 * Type macros.
 */
#define HUD_TYPE_UNITY_VOICE                  (hud_unity_voice_get_type ())
#define HUD_UNITY_VOICE(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), HUD_TYPE_UNITY_VOICE, HudUnityVoice))
#define HUD_IS_UNITY_VOICE(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HUD_TYPE_UNITY_VOICE))
#define HUD_UNITY_VOICE_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), HUD_TYPE_UNITY_VOICE, HudUnityVoiceClass))
#define HUD_IS_UNITY_VOICE_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), HUD_TYPE_UNITY_VOICE))
#define HUD_UNITY_VOICE_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), HUD_TYPE_UNITY_VOICE, HudUnityVoiceClass))

typedef struct _HudUnityVoice        HudUnityVoice;

typedef struct _HudQueryIfaceComCanonicalHudQuery HudQueryIfaceComCanonicalHudQuery;
typedef struct _HudSource         HudSource;

/* used by HUD_TYPE_UNITY_VOICE */
GType hud_unity_voice_get_type (void);

/*
 * Method definitions.
 */

HudUnityVoice * hud_unity_voice_new (HudQueryIfaceComCanonicalHudQuery *skel,
    const gchar *device, GError **error);

#endif /* __HUD_UNITY_VOICE_H__ */
