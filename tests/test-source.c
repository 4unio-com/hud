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

#define LOADER_NAME  "test.json.loader"
#define LOADER_PATH  "/test/json/loader"

#include "hudsettings.h"
#include "hudquery.h"
#include "hudtoken.h"
#include "hudsource.h"
#include "hudsourcelist.h"
#include "huddbusmenucollector.h"

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

/* If we can't get the name, we should error the test */
static gboolean
name_timeout (gpointer user_data)
{
  g_error("Unable to get name");
  return FALSE;
}

static void
start_dbusmenu_mock_app (DbusTestService ** service, GDBusConnection ** session, const gchar * jsonfile)
{
  *service = dbus_test_service_new(NULL);

  /* Loader */
  DbusTestProcess * loader = dbus_test_process_new(DBUSMENU_JSON_LOADER);
  dbus_test_process_append_param(loader, LOADER_NAME);
  dbus_test_process_append_param(loader, LOADER_PATH);
  dbus_test_process_append_param(loader, jsonfile);
  dbus_test_task_set_name(DBUS_TEST_TASK(loader), "JSON Loader");
  dbus_test_service_add_task(*service, DBUS_TEST_TASK(loader));
  g_object_unref(loader);

  /* Dummy */
  DbusTestTask * dummy = dbus_test_task_new();
  dbus_test_task_set_wait_for(dummy, LOADER_NAME);
  dbus_test_service_add_task(*service, dummy);
  g_object_unref(dummy);

  /* Setup timeout */
  guint timeout_source = g_timeout_add_seconds(2, name_timeout, NULL);

  /* Get loader up and running and us on that bus */
  g_debug("Starting up Dbusmenu Loader");
  dbus_test_service_start_tasks(*service);

  /* Cleanup timeout */
  g_source_remove(timeout_source);

  /* Set us not to exit when the service goes */
  *session = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, NULL);
  g_dbus_connection_set_exit_on_close(*session, FALSE);

  return;
}

static void
make_assertion (HudSource *source, const gchar *search,
    const gchar *appstack, const gchar *path, const gchar *name,
    const gchar **expected_rows, const guint32 *expected_distances,
    const guint expected_count)
{
  HudQuery *query;
  guint row;

  g_debug ("query: %s", search);

  query = hud_query_new (source, search, 1u << 30);

  g_assert_cmpstr(hud_query_get_appstack_name(query), ==, appstack);
  g_assert_cmpstr(hud_query_get_path(query), ==, path);
  g_assert_cmpstr(hud_query_get_results_name(query), ==, name);

  DeeModel *model = hud_query_get_results_model(query);

  g_assert_cmpint(dee_model_get_n_rows(model), ==, expected_count);

  for(row = 0; row < expected_count ; row++)
  {
    DeeModelIter* iter = dee_model_get_iter_at_row(model, row);

    g_debug("Result: %s", dee_model_get_string(model, iter, 1));
    g_debug("Expected: %s", expected_rows[row]);
    g_debug("Distance: %d", dee_model_get_uint32(model, iter, 6));
    g_debug("Exp Distance: %d", expected_distances[row]);

    g_assert_cmpstr(dee_model_get_string(model, iter, 1), ==, expected_rows[row]);
    g_assert_cmpint(dee_model_get_uint32(model, iter, 6), ==, expected_distances[row]);
  }

  g_object_unref (query);
}

/* Timeout on our loop */
static gboolean
test_menus_timeout (gpointer user_data)
{
  GMainLoop * loop = (GMainLoop *)user_data;
  g_main_loop_quit(loop);
  return FALSE;
}

static void
test_hud_query (void)
{
  DbusTestService * service = NULL;
  GDBusConnection * session = NULL;

  start_dbusmenu_mock_app (&service, &session, JSON_INPUT);

  HudDbusmenuCollector * collector = hud_dbusmenu_collector_new_for_endpoint (
      "test-id", "Prefix", "no-icon", 0, /* penalty */
      LOADER_NAME, LOADER_PATH);
  g_assert(collector != NULL);
  g_assert(HUD_IS_DBUSMENU_COLLECTOR(collector));

  GMainLoop * temploop = g_main_loop_new (NULL, FALSE);
  g_timeout_add (100, test_menus_timeout, temploop);
  g_main_loop_run (temploop);
  g_main_loop_unref (temploop);

  HudSourceList *source_list = hud_source_list_new();
  hud_source_list_add(source_list, HUD_SOURCE(collector));

  hud_source_use(HUD_SOURCE(source_list));

  {
    gchar *search = "ash";
    const gchar *expected[5] = { "any rash", "stray slash", "swift sad", "itch step", "mess strand" };
    const guint32 expected_distances[5] = { 10, 20, 22, 23, 25 };
    const gchar *appstack = "com.canonical.hud.query0.appstack";
    const gchar *path = "/com/canonical/hud/query0";
    const gchar *name = "com.canonical.hud.query0.results";
    make_assertion (HUD_SOURCE(source_list), search, appstack, path, name, expected, expected_distances, 5);
  }

  {
    gchar *search = "mess";
    const gchar *expected[1] = { "mess strand"};
    const guint32 expected_distances[1] = { 1 };
    const gchar *appstack = "com.canonical.hud.query1.appstack";
    const gchar *path = "/com/canonical/hud/query1";
    const gchar *name = "com.canonical.hud.query1.results";
    make_assertion (HUD_SOURCE(source_list), search, appstack, path, name, expected, expected_distances, 1);
  }

  {
    gchar *search = "dare";
    const gchar *expected[2] = { "mess strand", "bowl"};
    const guint32 expected_distances[2] = { 2, 30 };
    const gchar *appstack = "com.canonical.hud.query2.appstack";
    const gchar *path = "/com/canonical/hud/query2";
    const gchar *name = "com.canonical.hud.query2.results";
    make_assertion (HUD_SOURCE(source_list), search, appstack, path, name, expected, expected_distances, 2);
  }

  hud_source_unuse (HUD_SOURCE(source_list) );

  g_object_unref (collector);
  g_object_unref (service);
  g_object_unref (session);
  g_object_unref (source_list);
}

int
main (int argc, char **argv)
{
  g_type_init ();

  g_test_init (&argc, &argv, NULL);
  g_test_add_func ("/hud/source", test_hud_query);

  return g_test_run ();
}
