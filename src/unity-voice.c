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

#include <gio/gunixoutputstream.h>
#include <glib/gstdio.h>
#include <string.h>

static const gchar* UNITY_VOICE_BUS_NAME = "com.canonical.Unity.Voice";
static const gchar* UNITY_VOICE_OBJECT_PATH = "/com/canonical/Unity/Voice";

static void
hud_unity_voice_on_loading( UnityVoiceIfaceComCanonicalUnityVoice * proxy, gpointer user_data );
static void
hud_unity_voice_on_listening( UnityVoiceIfaceComCanonicalUnityVoice * proxy, gpointer user_data );
static void
hud_unity_voice_on_heard_something( UnityVoiceIfaceComCanonicalUnityVoice * proxy, gpointer user_data );

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

  glong loading_sig;
  glong listening_sig;
  glong heard_something_sig;

  GMainLoop * voice_loop;

  gchar **query_result;
  GError **query_error;
  gboolean query_success;
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

  g_signal_handler_disconnect(G_OBJECT(self->proxy), self->loading_sig);
  g_signal_handler_disconnect(G_OBJECT(self->proxy), self->listening_sig);
  g_signal_handler_disconnect(G_OBJECT(self->proxy), self->heard_something_sig);

  g_main_loop_unref (self->voice_loop);

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

  if (self->proxy == NULL) {
    g_warning("Unity-Voice proxy failed to initialize");
    g_set_error_literal (error, hud_unity_voice_error_quark (),
        HUD_VOICE_INITIALISATION_ERROR,
        "Unity-Voice proxy failed to initialize");
    return NULL;
  }

  self->loading_sig = g_signal_connect(G_OBJECT(self->proxy), "loading",
      G_CALLBACK(hud_unity_voice_on_loading), self->skel);
  self->listening_sig = g_signal_connect(G_OBJECT(self->proxy), "listening",
      G_CALLBACK(hud_unity_voice_on_listening), self->skel);
  self->heard_something_sig = g_signal_connect(G_OBJECT(self->proxy), "heard_something",
      G_CALLBACK(hud_unity_voice_on_heard_something), self->skel);

  self->voice_loop = g_main_loop_new (NULL, FALSE);

  self->query_result = NULL;
  self->query_error = NULL;
  self->query_success = FALSE;

  return self;
}

static GVariant*
hud_unity_voice_build_commands_variant( GList *items )
{
  GVariant* commands;
  GVariantBuilder builder;
  gboolean builder_valid = FALSE;

  // use builder to construct "aas" of commands
  g_variant_builder_init( &builder, G_VARIANT_TYPE_ARRAY );
  for( ; items != NULL; items = g_list_next( items ) )
  {
    GVariant* variant;
    GVariantBuilder as_builder;

    // get current item's command
    HudItem* item = ( HudItem* ) items->data;
    const gchar * command = hud_item_get_command( item );

    // continue only if there is a command available
    if (strcmp (command, "No Command") != 0)
    {
      gchar ** command_words = g_strsplit( command, " ", -1 );

      // use as_builder to construct a command "as" from command_words
      g_variant_builder_init( &as_builder, G_VARIANT_TYPE_STRING_ARRAY );
      for( int i = 0; command_words[i] != NULL ; i++ )
      {
        g_variant_builder_add_value( &as_builder, g_variant_new_string ( command_words[i] ) );
      }

      g_strfreev( command_words );

      variant = g_variant_builder_end( &as_builder );

      // add command "as" to commands "aas"
      g_variant_builder_add_value( &builder, variant );
      builder_valid = TRUE;
    }
  }

  if( !builder_valid )
  {
    return NULL;
  }

  commands = g_variant_builder_end( &builder );
  return commands;
}

static void
hud_unity_voice_on_loading( UnityVoiceIfaceComCanonicalUnityVoice * proxy, gpointer user_data )
{
  // patch signal through to HUD
  HudQueryIfaceComCanonicalHudQuery* skel = (HudQueryIfaceComCanonicalHudQuery*) user_data;
  hud_query_iface_com_canonical_hud_query_emit_voice_query_loading(
      HUD_QUERY_IFACE_COM_CANONICAL_HUD_QUERY( skel ) );
}

static void
hud_unity_voice_on_listening( UnityVoiceIfaceComCanonicalUnityVoice * proxy, gpointer user_data )
{
  // patch signal through to HUD
  HudQueryIfaceComCanonicalHudQuery* skel = (HudQueryIfaceComCanonicalHudQuery*) user_data;
  hud_query_iface_com_canonical_hud_query_emit_voice_query_listening(
      HUD_QUERY_IFACE_COM_CANONICAL_HUD_QUERY( skel ) );
}

static void
hud_unity_voice_on_heard_something ( UnityVoiceIfaceComCanonicalUnityVoice * proxy, gpointer user_data )
{
  // patch signal through to HUD
  HudQueryIfaceComCanonicalHudQuery* skel = (HudQueryIfaceComCanonicalHudQuery*) user_data;
  hud_query_iface_com_canonical_hud_query_emit_voice_query_heard_something(
      HUD_QUERY_IFACE_COM_CANONICAL_HUD_QUERY( skel ) );
}

static void
hud_unity_voice_listen_callback( GObject *source, GAsyncResult *async_result, gpointer user_data )
{
  HudUnityVoice *self = HUD_UNITY_VOICE( user_data );

  // attempt to retrieve listen result from proxy
  if( unity_voice_iface_com_canonical_unity_voice_call_listen_finish( self->proxy, self->query_result, async_result, self->query_error ) )
  {
    self->query_success = TRUE;
  }

  g_main_loop_quit( self->voice_loop );
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

  GVariant* commands = hud_unity_voice_build_commands_variant( items );
  if (commands == NULL) {
    *result = NULL;
    g_set_error_literal (error, hud_unity_voice_error_quark(), HUD_VOICE_HUD_STATE_ERROR,
            "No valid commands found in items supplied");
    return FALSE;
  }

  g_list_free_full(items, g_object_unref);

  // initialise query state members
  self->query_success = FALSE;
  self->query_error = error;
  self->query_result = result;

  // send commands via "listen" call to unity-voice API on DBus
  unity_voice_iface_com_canonical_unity_voice_call_listen( self->proxy, commands, NULL,
      hud_unity_voice_listen_callback, self );

  // run voice loop
  g_main_loop_run( self->voice_loop );

  // clear query state members
  self->query_result = NULL;
  self->query_error = NULL;

  return self->query_success;
}
