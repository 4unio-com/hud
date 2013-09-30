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

#define G_LOG_DOMAIN "hud-unity-voice"

#include "voice.h"
#include "unity-voice.h"
#include "hud-query-iface.h"
#include "source.h"
#include "pronounce-dict.h"

#include <gio/gunixoutputstream.h>
#include <glib/gstdio.h>

static GQuark
hud_unity_voice_error_quark(void)
{
  static GQuark quark = 0;
  if (quark == 0)
    quark = g_quark_from_static_string ("hud-unity-voice-error-quark");
  return quark;
}

static GRegex *
hud_unity_voice_alphanumeric_regex_new (void)
{
  GRegex *alphanumeric_regex = NULL;

  GError *error = NULL;
  alphanumeric_regex = g_regex_new("…|\\.\\.\\.", 0, 0, &error);
  if (alphanumeric_regex == NULL) {
    g_error("Compiling regex failed: [%s]", error->message);
    g_error_free(error);
  }

  return alphanumeric_regex;
}

struct _HudUnityVoice
{
  GObject parent_instance;

  HudQueryIfaceComCanonicalHudQuery *skel;
  GRegex * alphanumeric_regex;
};

typedef GObjectClass HudUnityVoiceClass;

static void hud_unity_voice_finalize (GObject *object);

static gboolean hud_unity_voice_query (HudVoice *self, HudSource *source,
    gchar **result, GError **error);

static void hud_unity_voice_iface_init (HudVoiceInterface *iface);

G_DEFINE_TYPE_WITH_CODE (HudUnityVoice, hud_unity_voice, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (HUD_TYPE_VOICE, hud_unity_voice_iface_init))

static void hud_unity_voice_iface_init (HudVoiceInterface *iface)
{
  iface->query = hud_unity_voice_query;
}

static void
hud_unity_voice_class_init (GObjectClass *klass)
{
  klass->finalize = hud_unity_voice_finalize;
}

static void
hud_unity_voice_init (HudUnityVoice *self)
{
  self->alphanumeric_regex = hud_unity_voice_alphanumeric_regex_new();
}

static void
hud_unity_voice_finalize (GObject *object)
{
  HudUnityVoice *self = HUD_UNITY_VOICE (object);

  g_clear_object(&self->skel);
  g_clear_pointer(&self->alphanumeric_regex, g_regex_unref);

  G_OBJECT_CLASS (hud_unity_voice_parent_class)
    ->finalize (object);
}

HudUnityVoice *
hud_unity_voice_new (HudQueryIfaceComCanonicalHudQuery *skel, const gchar *device, GError **error)
{
  HudUnityVoice *self = g_object_new (HUD_TYPE_UNITY_VOICE, NULL);
  self->skel = g_object_ref(skel);

  return self;
}

static gboolean
hud_unity_voice_query (HudVoice *voice, HudSource *source, gchar **result, GError **error)
{
  g_return_val_if_fail(HUD_IS_UNITY_VOICE(voice), FALSE);

  if (source == NULL) {
    /* No active window, that's fine, but we'll just move on */
    *result = NULL;
    g_set_error_literal (error, hud_unity_voice_error_quark(), 0, "Active source is null");
    return FALSE;
  }

  GList *items = hud_source_get_items(source);
  if (items == NULL) {
    /* The active window doesn't have items, that's cool.  We'll move on. */
    *result = NULL;
    return TRUE;
  }

  gboolean success = TRUE;

  g_list_free_full(items, g_object_unref);

  return success;
}
