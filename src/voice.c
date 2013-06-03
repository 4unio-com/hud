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

#define G_LOG_DOMAIN "hudvoice"

#include "hudvoice.h"
#include "hudsphinx.h"
#include "hudjulius.h"

/**
 * HudVoiceace:
 * @g_iface: the #GTypeInterface
 * @query: virtual function pointer for hud_voice_query()
 *
 * This is the interface vtable for #HudVoice.
 **/

G_DEFINE_INTERFACE (HudVoice, hud_voice, G_TYPE_OBJECT)

static void
hud_voice_default_init (HudVoiceInterface *iface)
{
}

HudVoice *
hud_voice_new (HudQueryIfaceComCanonicalHudQuery *skel, const gchar *device,
    GError **error)
{
  if (hud_julius_is_installed())
    return HUD_VOICE(hud_julius_new (skel));

  return HUD_VOICE(hud_sphinx_new (skel, device, error));
}


/**
 * hud_voice_query:
 * @voice: a #HudVoice
 * @source: a #HudSource to interrogate #HudItem s from
 * @result: the text result of the voice query will be returned here
 * @error: if the method returns FALSE, an error will be provdided here
 *
 * Run a voice query against the given source.
 * 
 * It is your resposibility to free the returned result and error.
 **/
gboolean
hud_voice_query (HudVoice *self, HudSource *source, gchar **result,
    GError **error)
{
  g_debug ("voice query on %s %p", G_OBJECT_TYPE_NAME (self), self);
  HudVoiceInterface *iface = HUD_VOICE_GET_IFACE (self);
  g_return_val_if_fail(iface != NULL, FALSE);
  g_return_val_if_fail(iface->query != NULL, FALSE);
  return iface->query(self, source, result, error);
}
