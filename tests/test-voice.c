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
#include <pulse/pulseaudio.h>
#include <pulse/glib-mainloop.h>

#include "hudvoice.h"
#include "hudsource.h"
#include "hudmanualsource.h"
#include "hud-query-iface.h"

typedef struct
{
  pa_mainloop *mainloop;
  pa_mainloop_api *api;
  pa_context *context;
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

static void
quit (int ret, pa_mainloop_api *mainloop_api)
{
  g_assert(mainloop_api);
  mainloop_api->quit (mainloop_api, ret);
}

static void
context_drain_complete (pa_context *c, void *userdata)
{
  pa_context_disconnect (c);
}

static void
drain (pa_context *context)
{
  pa_operation *o;

  if (!(o = pa_context_drain (context, context_drain_complete, NULL )))
  {
    g_debug("DRAIN: disconnecting");
    pa_context_disconnect (context);
  }
  else
  {
    g_debug("DRAIN: unreffing");
    pa_operation_unref (o);
  }
}

static void
index_callback (pa_context *c, uint32_t idx, void *userdata)
{
  g_assert(idx != PA_INVALID_INDEX);

  HudTestVoice *test_voice = (HudTestVoice *) userdata;
  g_debug("index_callback: loaded module with index [%d]", idx);
  test_voice->module_index = idx;

  drain (c);
}

static void
success_callback (pa_context *c, int success, void *userdata)
{
  HudTestVoice *test_voice = (HudTestVoice *) userdata;
  test_voice->operation_success = success;
  g_debug(
      "index_callback_unload: unloaded module with index [%d]", test_voice->module_index);
  test_voice->module_index = PA_INVALID_INDEX;

  drain (c);
}

static void
context_state_callback_load_module (pa_context *context,
    void *userdata)
{
  HudTestVoice *test_voice = (HudTestVoice *) userdata;

  gchar *arguments = NULL;

  switch (pa_context_get_state (context))
  {
  case PA_CONTEXT_CONNECTING:
  case PA_CONTEXT_AUTHORIZING:
  case PA_CONTEXT_SETTING_NAME:
    break;

  case PA_CONTEXT_READY:
    arguments = g_strdup_printf (
        "source_name=%s file=%s channels=1 format=s16le rate=16000",
        test_voice->name, test_voice->path);
    g_debug("Loading module with arguments=[%s]", arguments);
    pa_operation_unref (
        pa_context_load_module (context, "module-pipe-source", arguments,
            index_callback, userdata));
    g_free (arguments);
    break;

  case PA_CONTEXT_TERMINATED:
    quit (0, test_voice->api);
    break;

  case PA_CONTEXT_FAILED:
  default:
    quit (1, test_voice->api);
    break;
  }
}

static void
context_state_callback_unload_module (pa_context *context,
    void *userdata)
{
  HudTestVoice *test_voice = (HudTestVoice *) userdata;

  switch (pa_context_get_state (context))
  {
  case PA_CONTEXT_CONNECTING:
  case PA_CONTEXT_AUTHORIZING:
  case PA_CONTEXT_SETTING_NAME:
    break;

  case PA_CONTEXT_READY:
    g_debug("unload module [%d]", test_voice->module_index);
    pa_operation_unref (
        pa_context_unload_module (context, test_voice->module_index,
            success_callback, userdata));
    break;

  case PA_CONTEXT_TERMINATED:
    quit (0, test_voice->api);
    break;

  case PA_CONTEXT_FAILED:
  default:
    quit (1, test_voice->api);
    break;
  }
}

void
unload_pipe_module (HudTestVoice* test_voice)
{
  test_voice->mainloop = pa_mainloop_new ();
  g_assert(test_voice->mainloop);

  test_voice->api = pa_mainloop_get_api (test_voice->mainloop);

  test_voice->context = pa_context_new (test_voice->api, "hud.test");
  g_assert(test_voice->context);

  pa_context_set_state_callback (test_voice->context,
      context_state_callback_unload_module, &*test_voice);

  g_assert_cmpint(pa_context_connect (test_voice->context, NULL, 0, NULL ), >=,
      0);

  gint ret = 0;
  g_assert_cmpint(pa_mainloop_run(test_voice->mainloop, &ret), >=, 0);
  g_assert_cmpuint(test_voice->module_index, ==, PA_INVALID_INDEX);
  g_assert(test_voice->operation_success);

  if (test_voice->context)
    pa_context_unref (test_voice->context);

  if (test_voice->mainloop)
  {
    pa_signal_done ();
    pa_mainloop_free (test_voice->mainloop);
  }
}

void
load_pipe_module (HudTestVoice* test_voice)
{
  {
    test_voice->mainloop = pa_mainloop_new ();
    g_assert(test_voice->mainloop);

    test_voice->api = pa_mainloop_get_api (test_voice->mainloop);

    test_voice->context = pa_context_new (test_voice->api, "hud.test");
    g_assert(test_voice->context);

    pa_context_set_state_callback (test_voice->context,
        context_state_callback_load_module, &*test_voice);

    g_assert_cmpint(pa_context_connect (test_voice->context, NULL, 0, NULL ),
        >=, 0);

    gint ret = 0;
    g_assert_cmpint(pa_mainloop_run(test_voice->mainloop, &ret), >=, 0);
    g_assert_cmpuint(test_voice->module_index, !=, PA_INVALID_INDEX);

    if (test_voice->context)
      pa_context_unref (test_voice->context);

    if (test_voice->mainloop)
    {
      pa_signal_done ();
      pa_mainloop_free (test_voice->mainloop);
    }
  }
}

void
play_sound (const gchar *name, const gchar *to)
{
  GError *error = NULL;
  const gchar *argv[] =
  { "./test-voice-sounds-play.sh", name, to, NULL };
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
  { NULL, NULL, NULL, FALSE, PA_INVALID_INDEX, NULL, NULL };

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
  }
  {
    HudStringList *tokens = hud_string_list_add_item ("menu", NULL );
    tokens = hud_string_list_add_item ("color balance", tokens);
    hud_manual_source_add (source, tokens, NULL, "shortcut2", TRUE);
  }
  {
    HudStringList *tokens = hud_string_list_add_item ("menu", NULL );
    tokens = hud_string_list_add_item ("open tab", tokens);
    hud_manual_source_add (source, tokens, NULL, "shortcut3", TRUE);
  }
  {
    HudStringList *tokens = hud_string_list_add_item ("menu", NULL );
    tokens = hud_string_list_add_item ("open terminal", tokens);
    hud_manual_source_add (source, tokens, NULL, "shortcut4", TRUE);
  }
  {
    HudStringList *tokens = hud_string_list_add_item ("menu", NULL );
    tokens = hud_string_list_add_item ("system settings", tokens);
    hud_manual_source_add (source, tokens, NULL, "shortcut5", TRUE);
  }

  load_pipe_module (&test_voice);

  test_sound (&test_voice, voice, source, "auto-adjust", "auto adjust");
  test_sound (&test_voice, voice, source, "color-balance", "color balance");
  test_sound (&test_voice, voice, source, "open-tab", "open tab");
  test_sound (&test_voice, voice, source, "open-terminal", "open terminal");
  test_sound (&test_voice, voice, source, "system-settings", "system settings");

  unload_pipe_module (&test_voice);

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
