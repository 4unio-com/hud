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

#define G_LOG_DOMAIN "test-source"

#include "hudsettings.h"
#include "hudquery.h"
#include "hudtoken.h"
#include "hudstringlist.h"
#include "hudsource.h"
#include "hudsourcelist.h"
#include "hudmanualsource.h"
#include "huddbusmenucollector.h"
#include "hud-query-iface.h"
#include "hudtestutils.h"

#include <glib-object.h>
#include <dee.h>
#include <libdbustest/dbus-test.h>

/* hardcode some parameters for reasons of determinism.
 */
HudSettings hud_settings = {
  .indicator_penalty = 50,
  .add_penalty = 10,
  .drop_penalty = 10,
  .end_drop_penalty = 1,
  .swap_penalty = 15,
  .max_distance = 30
};

typedef struct
{
  GDBusConnection *session;
  const gchar *object_path;
  const gchar *query;
} TestSourceThreadData;

static HudQueryIfaceComCanonicalHudQuery*
test_source_create_proxy (TestSourceThreadData* thread_data)
{
  GError *error = NULL;
  GDBusConnection *session = G_DBUS_CONNECTION(thread_data->session);
  const gchar* name = g_dbus_connection_get_unique_name (session);

  HudQueryIfaceComCanonicalHudQuery *proxy =
      hud_query_iface_com_canonical_hud_query_proxy_new_sync (session,
          G_DBUS_PROXY_FLAGS_NONE, name, thread_data->object_path, NULL,
          &error);
  if (error != NULL )
  {
    g_warning("%s %s\n", "The request failed:", error->message);
    g_error_free (error);
    return NULL ;
  }

  return proxy;
}

static gpointer
test_source_call_close_query (gpointer user_data)
{
  HudQueryIfaceComCanonicalHudQuery *proxy = test_source_create_proxy (
      (TestSourceThreadData*) user_data);

  GError *error = NULL;
  if (!hud_query_iface_com_canonical_hud_query_call_close_query_sync (proxy,
      NULL, &error))
  {
    g_warning("%s %s\n", "The request failed:", error->message);
    g_error_free (error);
  }

  g_object_unref (proxy);

  return FALSE;
}

static gpointer
test_source_call_update_query (gpointer user_data)
{
  TestSourceThreadData* thread_data = (TestSourceThreadData*) user_data;
  HudQueryIfaceComCanonicalHudQuery *proxy = test_source_create_proxy (
      thread_data);

  gint model_revision;
  GError *error = NULL;
  if (!hud_query_iface_com_canonical_hud_query_call_update_query_sync (proxy,
      thread_data->query, &model_revision, NULL, &error))
  {
    g_warning("%s %s\n", "The request failed:", error->message);
    g_error_free (error);
  }

  g_object_unref (proxy);

  return FALSE;
}

static void
test_source_make_assertions (HudQuery* query, const gchar *appstack,
    const gchar *path, const gchar *name, const gchar **expected_rows,
    const guint32 *expected_distances, const guint expected_count)
{
  guint row;

  g_assert_cmpstr(hud_query_get_appstack_name(query), ==, appstack);
  g_assert_cmpstr(hud_query_get_path(query), ==, path);
  g_assert_cmpstr(hud_query_get_results_name(query), ==, name);

  DeeModel* model = hud_query_get_results_model (query);
  g_assert_cmpint(dee_model_get_n_rows(model), ==, expected_count);

  for (row = 0; row < expected_count; row++)
  {
    DeeModelIter* iter = dee_model_get_iter_at_row (model, row);

    g_debug("Result: %s", dee_model_get_string(model, iter, 1));
    g_debug("Expected: %s", expected_rows[row]);
    g_debug("Distance: %d", dee_model_get_uint32(model, iter, 6));
    g_debug("Exp Distance: %d", expected_distances[row]);

    g_assert_cmpstr(dee_model_get_string(model, iter, 1), ==, expected_rows[row]);
    g_assert_cmpint(dee_model_get_uint32(model, iter, 6), ==, expected_distances[row]);
  }
}

static HudQuery*
test_source_create_query (GDBusConnection *session, HudSource *source, const gchar *search)
{
  g_debug ("query: [%s], on [%s]", search, g_dbus_connection_get_unique_name(session));
  return hud_query_new (source, search, 1u << 30, session);
}

static void
test_hud_query_sequence ()
{
  DbusTestService * service = NULL;
  GDBusConnection * session = NULL;

  hud_test_utils_start_dbusmenu_mock_app (&service, &session, JSON_INPUT);

  HudDbusmenuCollector *collector = hud_dbusmenu_collector_new_for_endpoint (
      "test-id", "Prefix", "no-icon", 0, /* penalty */
      HUD_TEST_UTILS_LOADER_NAME, HUD_TEST_UTILS_LOADER_PATH);
  g_assert(collector != NULL);
  g_assert(HUD_IS_DBUSMENU_COLLECTOR(collector));

  hud_test_utils_process_mainloop (100);

  HudManualSource *manual_source = hud_manual_source_new();

  HudSourceList *source_list = hud_source_list_new();
  hud_source_list_add(source_list, HUD_SOURCE(collector));
  hud_source_list_add(source_list, HUD_SOURCE(manual_source));

  hud_source_use(HUD_SOURCE(source_list));

  {
    gchar *search = "ash";
    const gchar *expected[5] = { "any rash", "stray slash", "swift sad", "itch step", "mess strand" };
    const guint32 expected_distances[5] = { 10, 20, 22, 23, 25 };
    const gchar *appstack = "com.canonical.hud.query0.appstack";
    const gchar *path = "/com/canonical/hud/query0";
    const gchar *name = "com.canonical.hud.query0.results";

    HudQuery *query = test_source_create_query (session, HUD_SOURCE(source_list), search);
    test_source_make_assertions (query, appstack, path, name, expected, expected_distances, 5);
    g_object_unref (query);
  }

  {
    gchar *search = "mess";
    const gchar *expected[1] = { "mess strand"};
    const guint32 expected_distances[1] = { 1 };
    const gchar *appstack = "com.canonical.hud.query1.appstack";
    const gchar *path = "/com/canonical/hud/query1";
    const gchar *name = "com.canonical.hud.query1.results";

    HudQuery *query = test_source_create_query (session, HUD_SOURCE(source_list), search);
    test_source_make_assertions (query, appstack, path, name, expected, expected_distances, 1);
    g_object_unref (query);
  }

  {
    gchar *search = "dare";
    const gchar *expected[2] = { "mess strand", "bowl"};
    const guint32 expected_distances[2] = { 2, 30 };
    const gchar *appstack = "com.canonical.hud.query2.appstack";
    const gchar *path = "/com/canonical/hud/query2";
    const gchar *name = "com.canonical.hud.query2.results";

    HudQuery *query = test_source_create_query (session, HUD_SOURCE(source_list), search);
    test_source_make_assertions (query, appstack, path, name, expected, expected_distances, 2);
    g_object_unref (query);
  }

  /* This time closing using dbus call */
  {
    gchar *search = "itch step";
    const gchar *expected[1] = { "itch step"};
    const guint32 expected_distances[1] = { 0 };
    const gchar *appstack = "com.canonical.hud.query3.appstack";
    const gchar *path = "/com/canonical/hud/query3";
    const gchar *name = "com.canonical.hud.query3.results";

    HudQuery *query = test_source_create_query (session, HUD_SOURCE(source_list), search);
    test_source_make_assertions (query, appstack, path, name, expected, expected_distances, 1);

    TestSourceThreadData thread_data = {session, path};
    GThread* thread = g_thread_new ("close_query", test_source_call_close_query, &thread_data);
    hud_test_utils_process_mainloop(100);
    g_thread_join(thread);
  }

  /* This time updating query using dbus call */
  {
    gchar *search = "ash";
    const gchar *expected[5] = { "any rash", "stray slash", "swift sad", "itch step", "mess strand" };
    const guint32 expected_distances[5] = { 10, 20, 22, 23, 25 };
    const gchar *appstack = "com.canonical.hud.query4.appstack";
    const gchar *path = "/com/canonical/hud/query4";
    const gchar *name = "com.canonical.hud.query4.results";

    HudQuery *query = test_source_create_query (session, HUD_SOURCE(source_list), search);
    test_source_make_assertions (query, appstack, path, name, expected, expected_distances, 5);

    TestSourceThreadData thread_data = {session, path, "dare"};
    GThread* thread = g_thread_new ("update_query", test_source_call_update_query, &thread_data);
    hud_test_utils_process_mainloop(100);
    g_thread_join(thread);

    const gchar *expected_after[2] = { "mess strand", "bowl"};
    const guint32 expected_distances_after[2] = { 2, 30 };

    test_source_make_assertions (query, appstack, path, name, expected_after, expected_distances_after, 2);

    g_object_unref (query);
  }

  /* Adding new data to the manual source */
  {
      gchar *search = "dare";
      const gchar *expected[2] = { "mess strand", "bowl"};
      const guint32 expected_distances[2] = { 2, 30 };
      const gchar *appstack = "com.canonical.hud.query5.appstack";
      const gchar *path = "/com/canonical/hud/query5";
      const gchar *name = "com.canonical.hud.query5.results";

      HudQuery *query = test_source_create_query (session, HUD_SOURCE(source_list), search);
      test_source_make_assertions (query, appstack, path, name, expected, expected_distances, 2);

      HudStringList *tokens = hud_string_list_add_item("extra", NULL);
      tokens = hud_string_list_add_item("something dare", tokens);
      hud_manual_source_add(manual_source, tokens, NULL, "shortcut1", NULL, NULL, TRUE);

      HudStringList *tokens2 = hud_string_list_add_item("extra", NULL);
      tokens2 = hud_string_list_add_item("something else darn", tokens2);
      hud_manual_source_add(manual_source, tokens2, NULL, "shortcut2", NULL, NULL, TRUE);

      hud_test_utils_process_mainloop(50);

      const gchar *expected_after[4] = { "something dare", "mess strand", "something else darn", "bowl"};
      const guint32 expected_distances_after[4] = { 0, 2, 11, 30 };

      test_source_make_assertions (query, appstack, path, name, expected_after, expected_distances_after, 4);

      g_object_unref (query);
    }

  hud_source_unuse (HUD_SOURCE(source_list) );

  g_object_unref (source_list);
  g_object_unref (collector);
  g_object_unref (manual_source);

  hud_test_utils_process_mainloop(100);

  dbus_test_service_stop(service);
  g_object_unref (service);
  g_object_unref (session);
}

static void
test_hud_query_sequence_counter_increment ()
{
  DbusTestService * service = NULL;
  GDBusConnection * session = NULL;

  hud_test_utils_start_dbusmenu_mock_app (&service, &session, JSON_INPUT);

  HudDbusmenuCollector *collector = hud_dbusmenu_collector_new_for_endpoint (
      "test-id", "Prefix", "no-icon", 0, /* penalty */
      HUD_TEST_UTILS_LOADER_NAME, HUD_TEST_UTILS_LOADER_PATH);
  g_assert(collector != NULL);
  g_assert(HUD_IS_DBUSMENU_COLLECTOR(collector));

  hud_test_utils_process_mainloop (100);

  HudManualSource *manual_source = hud_manual_source_new();

  HudSourceList *source_list = hud_source_list_new();
  hud_source_list_add(source_list, HUD_SOURCE(collector));
  hud_source_list_add(source_list, HUD_SOURCE(manual_source));

  hud_source_use(HUD_SOURCE(source_list));

  {
    gchar *search = "ash";
    const gchar *expected[5] = { "any rash", "stray slash", "swift sad", "itch step", "mess strand" };
    const guint32 expected_distances[5] = { 10, 20, 22, 23, 25 };
    const gchar *appstack = "com.canonical.hud.query6.appstack";
    const gchar *path = "/com/canonical/hud/query6";
    const gchar *name = "com.canonical.hud.query6.results";

    HudQuery *query = test_source_create_query (session, HUD_SOURCE(source_list), search);
    test_source_make_assertions (query, appstack, path, name, expected, expected_distances, 5);
    g_object_unref (query);
  }

  {
    gchar *search = "mess";
    const gchar *expected[1] = { "mess strand"};
    const guint32 expected_distances[1] = { 1 };
    const gchar *appstack = "com.canonical.hud.query7.appstack";
    const gchar *path = "/com/canonical/hud/query7";
    const gchar *name = "com.canonical.hud.query7.results";

    HudQuery *query = test_source_create_query (session, HUD_SOURCE(source_list), search);
    test_source_make_assertions (query, appstack, path, name, expected, expected_distances, 1);
    g_object_unref (query);
  }

  {
    gchar *search = "dare";
    const gchar *expected[2] = { "mess strand", "bowl"};
    const guint32 expected_distances[2] = { 2, 30 };
    const gchar *appstack = "com.canonical.hud.query8.appstack";
    const gchar *path = "/com/canonical/hud/query8";
    const gchar *name = "com.canonical.hud.query8.results";

    HudQuery *query = test_source_create_query (session, HUD_SOURCE(source_list), search);
    test_source_make_assertions (query, appstack, path, name, expected, expected_distances, 2);
    g_object_unref (query);
  }

  /* This time closing using dbus call */
  {
    gchar *search = "itch step";
    const gchar *expected[1] = { "itch step"};
    const guint32 expected_distances[1] = { 0 };
    const gchar *appstack = "com.canonical.hud.query9.appstack";
    const gchar *path = "/com/canonical/hud/query9";
    const gchar *name = "com.canonical.hud.query9.results";

    HudQuery *query = test_source_create_query (session, HUD_SOURCE(source_list), search);
    test_source_make_assertions (query, appstack, path, name, expected, expected_distances, 1);

    TestSourceThreadData thread_data = {session, path};
    GThread* thread = g_thread_new ("close_query", test_source_call_close_query, &thread_data);
    hud_test_utils_process_mainloop(100);
    g_thread_join(thread);
  }

  /* This time updating query using dbus call */
  {
    gchar *search = "ash";
    const gchar *expected[5] = { "any rash", "stray slash", "swift sad", "itch step", "mess strand" };
    const guint32 expected_distances[5] = { 10, 20, 22, 23, 25 };
    const gchar *appstack = "com.canonical.hud.query10.appstack";
    const gchar *path = "/com/canonical/hud/query10";
    const gchar *name = "com.canonical.hud.query10.results";

    HudQuery *query = test_source_create_query (session, HUD_SOURCE(source_list), search);
    test_source_make_assertions (query, appstack, path, name, expected, expected_distances, 5);

    TestSourceThreadData thread_data = {session, path, "dare"};
    GThread* thread = g_thread_new ("update_query", test_source_call_update_query, &thread_data);
    hud_test_utils_process_mainloop(100);
    g_thread_join(thread);

    const gchar *expected_after[2] = { "mess strand", "bowl"};
    const guint32 expected_distances_after[2] = { 2, 30 };

    test_source_make_assertions (query, appstack, path, name, expected_after, expected_distances_after, 2);

    g_object_unref (query);
  }

  /* Adding new data to the manual source */
  {
      gchar *search = "dare";
      const gchar *expected[2] = { "mess strand", "bowl"};
      const guint32 expected_distances[2] = { 2, 30 };
      const gchar *appstack = "com.canonical.hud.query11.appstack";
      const gchar *path = "/com/canonical/hud/query11";
      const gchar *name = "com.canonical.hud.query11.results";

      HudQuery *query = test_source_create_query (session, HUD_SOURCE(source_list), search);
      test_source_make_assertions (query, appstack, path, name, expected, expected_distances, 2);

      HudStringList *tokens = hud_string_list_add_item("extra", NULL);
      tokens = hud_string_list_add_item("something dare", tokens);
      hud_manual_source_add(manual_source, tokens, NULL, "shortcut1", NULL, NULL, TRUE);

      HudStringList *tokens2 = hud_string_list_add_item("extra", NULL);
      tokens2 = hud_string_list_add_item("something else darn", tokens2);
      hud_manual_source_add(manual_source, tokens2, NULL, "shortcut2", NULL, NULL, TRUE);

      hud_test_utils_process_mainloop(50);

      const gchar *expected_after[4] = { "something dare", "mess strand", "something else darn", "bowl"};
      const guint32 expected_distances_after[4] = { 0, 2, 11, 30 };

      test_source_make_assertions (query, appstack, path, name, expected_after, expected_distances_after, 4);

      g_object_unref (query);
    }

  hud_source_unuse (HUD_SOURCE(source_list) );

  g_object_unref (source_list);
  g_object_unref (collector);
  g_object_unref (manual_source);

  hud_test_utils_process_mainloop(100);

  dbus_test_service_stop(service);
  g_object_unref (service);
  g_object_unref (session);
}

int
main (int argc, char **argv)
{
  g_type_init ();

  g_test_init (&argc, &argv, NULL);
  g_test_add_func ("/hud/source/query_sequence", test_hud_query_sequence);
  g_test_add_func ("/hud/source/query_sequence_counter_increment", test_hud_query_sequence_counter_increment);

  return g_test_run ();
}
