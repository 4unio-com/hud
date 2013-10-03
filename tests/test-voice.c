/*
 Copyright 2012 Canonical Ltd.

 This program is free software: you can redistribute it and/or modify it
 under the terms of the GNU General Public License version 3, as published
 by the Free Software Foundation.

 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranties of
 MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 PURPOSE.  See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along
 with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib-object.h>
#include <glib/gstdio.h>

#include "hud-query-iface.h"
#include <libdbustest/dbus-test.h>
#include "manual-source.h"
#include "source.h"
#include "test-utils.h"
#include "voice.h"

static const gchar* UNITY_VOICE_BUS_NAME = "com.canonical.Unity.Voice";
static const gchar* UNITY_VOICE_OBJECT_PATH = "/com/canonical/Unity/Voice";
static const gchar* UNITY_VOICE_INTERFACE = "com.canonical.Unity.Voice";

typedef struct
{
  gboolean operation_success;
  guint module_index;
  gchar *name;
  gchar *path;
} HudTestVoice;

static void
test_hud_voice_construct ()
{
  HudQueryIfaceComCanonicalHudQuery *skel =
      hud_query_iface_com_canonical_hud_query_skeleton_new ();

  GError *error = NULL;
  HudVoice* voice = hud_voice_new (skel, NULL, &error);
  g_assert(voice != NULL);
  g_assert(error == NULL);

  g_object_unref(voice);
  g_object_unref(skel);
}

static void
test_hud_voice_query_null_source ()
{
  HudQueryIfaceComCanonicalHudQuery *skel =
      hud_query_iface_com_canonical_hud_query_skeleton_new ();

  GError *error = NULL;
  HudVoice* voice = hud_voice_new (skel, NULL, &error);
  g_assert(voice != NULL);
  g_assert(error == NULL);

  gchar *result = NULL;
  g_assert(!hud_voice_query(voice, NULL, &result, &error));
  g_assert(result == NULL);
  g_assert(error != NULL);

  g_object_unref(voice);
  g_error_free(error);
  g_object_unref(skel);
}

static void
test_hud_voice_query_empty_source ()
{
  HudQueryIfaceComCanonicalHudQuery *skel =
      hud_query_iface_com_canonical_hud_query_skeleton_new ();

  GError *error = NULL;
  HudVoice* voice = hud_voice_new (skel, NULL, &error);
  g_assert(voice != NULL);
  g_assert(error == NULL);

  HudManualSource* source = hud_manual_source_new ("test_id", "test_icon");

  gchar *result = NULL;
  g_assert(hud_voice_query(voice, HUD_SOURCE(source), &result, &error));
  g_assert(error == NULL);

  g_assert(result == NULL);

  g_object_unref(source);
  g_object_unref(voice);
  g_object_unref(skel);
}

void
test_sound (const HudTestVoice* test_voice, HudVoice* voice,
    HudManualSource* source, const gchar* name, const gchar *expected)
{
  // tell unity-voice dbus mock that we expect "expected"?

  gchar *result = NULL;
  GError *error = NULL;

  // call query and obtain result
  g_assert(hud_voice_query(voice, HUD_SOURCE(source), &result, &error));

  g_assert(error == NULL);
  g_assert(result != NULL);
  g_assert_cmpstr(result, ==, expected);
  g_debug("QUERY RESULT [%s]", result);

  g_free (result);
}

static void
test_hud_voice_query ()
{
  DbusTestService *service = dbus_test_service_new (NULL);

  hud_test_utils_dbus_mock_start (service, UNITY_VOICE_BUS_NAME,
      UNITY_VOICE_OBJECT_PATH, UNITY_VOICE_INTERFACE);

  GDBusConnection *connection = hud_test_utils_mock_dbus_connection_new (service,
      UNITY_VOICE_BUS_NAME, NULL );

  hud_test_utils_process_mainloop (300);

  {
    dbus_mock_add_method (connection,
        UNITY_VOICE_BUS_NAME, UNITY_VOICE_OBJECT_PATH,
        UNITY_VOICE_INTERFACE, "listen", "aas", "s",
          "ret = 'auto adjust'");
  }

  HudQueryIfaceComCanonicalHudQuery *skel =
      hud_query_iface_com_canonical_hud_query_skeleton_new ();

  GError *error = NULL;
  HudVoice* voice = hud_voice_new (skel, NULL, &error);
  g_assert(voice != NULL);
  g_assert(error == NULL);

  HudManualSource* source = hud_manual_source_new ("test_id", "test_icon");

  HudStringList *tokens = hud_string_list_add_item ("menu", NULL );
  tokens = hud_string_list_add_item ("auto adjust", tokens);
  hud_manual_source_add (source, tokens, NULL, "shortcut1", TRUE);
  hud_string_list_unref(tokens);

  HudTestVoice test_voice =
  { FALSE, 0, NULL, NULL };

  test_sound (&test_voice, voice, source, "auto-adjust", "auto adjust");

  g_free (test_voice.name);
  g_free (test_voice.path);

  g_object_unref (source);
  g_object_unref (voice);
  g_object_unref (skel);

  g_object_unref (connection);
  g_object_unref (service);
}

static void
test_string_list_suite ()
{
  g_test_add_func ("/hud/voice/construct", test_hud_voice_construct);
  g_test_add_func ("/hud/voice/query_null_source", test_hud_voice_query_null_source);
  g_test_add_func ("/hud/voice/query_empty_source", test_hud_voice_query_empty_source);
  g_test_add_func ("/hud/voice/query", test_hud_voice_query);
}

gint main (gint argc, gchar * argv[])
{
#ifndef GLIB_VERSION_2_36
  g_type_init ();
#endif
  g_test_init (&argc, &argv, NULL );
  test_string_list_suite ();
  return g_test_run ();
}
