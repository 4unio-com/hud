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
#include "unity-voice-iface.h"
#include "hud-query-iface.h"
#include "source.h"
#include "pronounce-dict.h"

#include <gio/gunixoutputstream.h>
#include <glib/gstdio.h>

static const gchar* UNITY_VOICE_BUS_NAME = "com.canonical.Unity.Voice";
static const gchar* UNITY_VOICE_OBJECT_PATH = "/com/canonical/Unity/Voice";

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
  UnityVoiceIfaceComCanonicalUnityVoice *proxy;
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
  g_clear_object(&self->proxy);
  g_clear_pointer(&self->alphanumeric_regex, g_regex_unref);

  G_OBJECT_CLASS (hud_unity_voice_parent_class)
    ->finalize (object);
}

HudUnityVoice *
hud_unity_voice_new (HudQueryIfaceComCanonicalHudQuery *skel, const gchar *device, GError **error)
{
  HudUnityVoice *self = g_object_new (HUD_TYPE_UNITY_VOICE, NULL);
  self->skel = g_object_ref(skel);

  // create proxy for unity-voice
  self->proxy = unity_voice_iface_com_canonical_unity_voice_proxy_new_for_bus_sync(
    G_BUS_TYPE_SESSION,
    G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES,
    UNITY_VOICE_BUS_NAME,
    UNITY_VOICE_OBJECT_PATH,
    NULL,
    error
  );

  return self;
}

static GVariant*
hud_unity_voice_build_commands_variant(GList *items)
{
  GVariant* commands;
  GVariantBuilder builder;

  // use builder to construct aas of commands
  g_variant_builder_init( &builder, G_VARIANT_TYPE_ARRAY );
  for( ; items != NULL; items = g_list_next( items ) )
  {
    GVariant* variant;
    GVariantBuilder as_builder;

    // get current item's token list
    HudItem* item = ( HudItem* ) items->data;

    // use as_builder to construct a command as from command
    g_variant_builder_init( &as_builder, G_VARIANT_TYPE_STRING_ARRAY );

    const gchar * command = hud_item_get_command( item );
    gchar ** command_list = g_strsplit( command, " ", -1 );

    for( int i = 0; command_list[i] != NULL ; i++ )
    {
      g_variant_builder_add_value( &as_builder, g_variant_new_string ( command_list[i] ) );
    }

    g_strfreev( command_list );

    variant = g_variant_builder_end( &as_builder );

    // add command as to aas commands
    g_variant_builder_add_value( &builder, variant );
  }

  commands = g_variant_builder_end( &builder );

  return commands;
}

static gboolean
hud_unity_voice_query (HudVoice *voice, HudSource *source, gchar **result, GError **error)
{
  g_return_val_if_fail(HUD_IS_UNITY_VOICE(voice), FALSE);
  HudUnityVoice *self = HUD_UNITY_VOICE(voice);

  if (source == NULL) {
    /* No active window, that's fine, but we'll just move on */
    *result = NULL;
    g_set_error_literal (error, hud_unity_voice_error_quark(), HUD_VOICE_HUD_STATE_ERROR,
            "Active source is null");
    return FALSE;
  }

  GList *items = hud_source_get_items(source);
  if (items == NULL) {
    /* The active window doesn't have items, that's cool.  We'll move on. */
    *result = NULL;
    return TRUE;
  }

  // send commands via "listen" call to unity-voice API on DBus
  GVariant* commands = hud_unity_voice_build_commands_variant( items );
  gboolean success = unity_voice_iface_com_canonical_unity_voice_call_listen_sync(self->proxy, commands, result, NULL, NULL);

  g_list_free_full(items, g_object_unref);

  return success;
}
