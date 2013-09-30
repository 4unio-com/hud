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

#include "voice.h"
#include "source.h"
#include "manual-source.h"
#include "hud-query-iface.h"

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

  g_error_free(error);
  g_object_unref(voice);
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
play_sound (const gchar *name, const gchar *to)
{
  GError *error = NULL;
  const gchar *argv[] =
  { TEST_VOICE_SOUNDS_PLAY, name, to, NULL };
  if (!g_spawn_async (NULL, (gchar **) argv, NULL, 0, NULL, NULL, NULL, &error))
  {
    g_error("%s", error->message);
    g_error_free (error);
  }
}

void
test_sound (const HudTestVoice* test_voice, HudVoice* voice,
    HudManualSource* source, const gchar* name, const gchar *expected)
{
  play_sound (name, test_voice->path);
  gchar *result = NULL;
  GError *error = NULL;
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
  HudQueryIfaceComCanonicalHudQuery *skel =
      hud_query_iface_com_canonical_hud_query_skeleton_new ();

  HudTestVoice test_voice =
  { FALSE, 0, NULL, NULL };

  GError *error = NULL;
  gint file_handle = g_file_open_tmp ("hud.test-voice-XXXXXX.source",
      &test_voice.path, &error);
  g_assert_cmpint(file_handle, !=, -1);
  g_assert(error == NULL);
  close (file_handle);
  g_remove (test_voice.path);
  test_voice.name = g_path_get_basename (test_voice.path);

  error = NULL;
  HudVoice* voice = hud_voice_new (skel, test_voice.name, &error);
  g_assert(voice != NULL);
  g_assert(error == NULL);

  HudManualSource* source = hud_manual_source_new ("test_id", "test_icon");

  {
    HudStringList *tokens = hud_string_list_add_item ("menu", NULL );
    tokens = hud_string_list_add_item ("auto adjust", tokens);
    hud_manual_source_add (source, tokens, NULL, "shortcut1", TRUE);
    hud_string_list_unref(tokens);
  }
  {
    HudStringList *tokens = hud_string_list_add_item ("menu", NULL );
    tokens = hud_string_list_add_item ("color balance", tokens);
    hud_manual_source_add (source, tokens, NULL, "shortcut2", TRUE);
    hud_string_list_unref(tokens);
  }
  {
    HudStringList *tokens = hud_string_list_add_item ("menu", NULL );
    tokens = hud_string_list_add_item ("open tab", tokens);
    hud_manual_source_add (source, tokens, NULL, "shortcut3", TRUE);
    hud_string_list_unref(tokens);
  }
  {
    HudStringList *tokens = hud_string_list_add_item ("menu", NULL );
    tokens = hud_string_list_add_item ("open terminal", tokens);
    hud_manual_source_add (source, tokens, NULL, "shortcut4", TRUE);
    hud_string_list_unref(tokens);
  }
  {
    HudStringList *tokens = hud_string_list_add_item ("menu", NULL );
    tokens = hud_string_list_add_item ("system settings", tokens);
    hud_manual_source_add (source, tokens, NULL, "shortcut5", TRUE);
    hud_string_list_unref(tokens);
  }

  test_sound (&test_voice, voice, source, "auto-adjust", "auto adjust");
  test_sound (&test_voice, voice, source, "color-balance", "color balance");
  test_sound (&test_voice, voice, source, "open-tab", "open tab");
  test_sound (&test_voice, voice, source, "open-terminal", "open terminal");
  test_sound (&test_voice, voice, source, "system-settings", "system settings");

  g_free (test_voice.name);
  g_free (test_voice.path);

  g_object_unref (source);
  g_object_unref (voice);
  g_object_unref (skel);
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
