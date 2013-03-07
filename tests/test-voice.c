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

#include "hudvoice.h"
#include "hudsource.h"
#include "hudmanualsource.h"
#include "hud-query-iface.h"

static void
test_hud_voice_construct ()
{
  HudQueryIfaceComCanonicalHudQuery *skel =
      hud_query_iface_com_canonical_hud_query_skeleton_new ();
  GError *error = NULL;
  HudVoice* voice = hud_voice_new (skel, &error);
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
  HudVoice* voice = hud_voice_new (skel, &error);
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
  HudVoice* voice = hud_voice_new (skel, &error);
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

static void
test_hud_voice_query ()
{
  HudQueryIfaceComCanonicalHudQuery *skel =
      hud_query_iface_com_canonical_hud_query_skeleton_new ();

  GError *error = NULL;
  HudVoice* voice = hud_voice_new (skel, &error);
  g_assert(voice != NULL);
  g_assert(error == NULL);

  HudManualSource* source = hud_manual_source_new ("test_id", "test_icon");

  {
    HudStringList *tokens = hud_string_list_add_item("menu", NULL);
    tokens = hud_string_list_add_item("open terminal", tokens);
    hud_manual_source_add(source, tokens, NULL, "shortcut1", TRUE);
  }
  {
    HudStringList *tokens = hud_string_list_add_item("menu", NULL);
    tokens = hud_string_list_add_item("open tab", tokens);
    hud_manual_source_add(source, tokens, NULL, "shortcut2", TRUE);
  }

  {
    gchar *result = NULL;
    error = NULL;
    g_assert(hud_voice_query(voice, HUD_SOURCE(source), &result, &error));
    g_assert(error == NULL);
    g_assert(result != NULL);
    g_assert_cmpstr(result, ==, "open terminal");
    g_debug("VOICE RESULT [%s]", result);
    g_free (result);
  }

  {
    gchar *result = NULL;
    error = NULL;
    g_assert(hud_voice_query(voice, HUD_SOURCE(source), &result, &error));
    g_assert(error == NULL);
    g_assert(result != NULL);
    g_assert_cmpstr(result, ==, "open tab");
    g_debug("VOICE RESULT [%s]", result);
    g_free (result);
  }

  g_object_unref(source);
  g_object_unref(voice);
  g_object_unref(skel);
}

static void
test_string_list_suite ()
{
  g_test_add_func ("/hud/voice/construct", test_hud_voice_construct);
  g_test_add_func ("/hud/voice/query_null_source", test_hud_voice_query_null_source);
  g_test_add_func ("/hud/voice/query_empty_source", test_hud_voice_query_empty_source);
  g_test_add_func ("/hud/voice/query", test_hud_voice_query);
}

gint
main (gint argc, gchar * argv[])
{
#ifndef GLIB_VERSION_2_36
  g_type_init ();
#endif
  g_test_init (&argc, &argv, NULL );
  test_string_list_suite ();
  return g_test_run ();
}
