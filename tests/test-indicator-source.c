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

#define G_LOG_DOMAIN "test-hudappindicatorsource"

#include "hudsettings.h"
#include "hudsource.h"
#include "hudtoken.h"
#include "hudindicatorsource.h"
#include "hudtestutils.h"

#include <glib-object.h>
#include <gio/gio.h>
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


static void
test_indicator_source_datetime ()
{
  const gchar *dbus_name = "com.canonical.indicator.datetime";
  const gchar *dbus_menu_path = "/com/canonical/indicator/datetime/menu";
//  const gchar *indicator_name = "indicator-datetime";

  DbusTestService *service = dbus_test_service_new (NULL);

  hud_test_utils_json_loader_start_full (service, dbus_name, dbus_menu_path,
      "./test-indicator-source-datetime.json");
  GDBusConnection *connection = hud_test_utils_mock_dbus_connection_new (service,
      dbus_name, NULL);

  HudIndicatorSource* source = hud_indicator_source_new (connection);
  hud_test_utils_process_mainloop (100);

  g_assert(source != NULL);
  g_assert(HUD_IS_INDICATOR_SOURCE(source));

  HudTokenList *search = hud_token_list_new_from_string ("date time");

  {
    GPtrArray *results = g_ptr_array_new_with_free_func(g_object_unref);
    hud_source_search(HUD_SOURCE(source), search, hud_test_utils_results_append_func, results);
    g_assert_cmpuint(results->len, ==, 1);
    hud_test_utils_source_assert_result (results, 0, "Date Time");
    g_ptr_array_free(results, TRUE);
  }

  hud_token_list_free(search);
  g_object_unref (source);
  dbus_test_service_stop(service);
  g_object_unref (service);
  g_object_unref (connection);
}

static void
test_indicator_source_session ()
{
  const gchar *dbus_name = "com.canonical.indicator.session";
  const gchar *dbus_menu_path = "/com/canonical/indicator/session/menu";
//  const gchar *indicator_name = "indicator-session-device";

  DbusTestService *service = dbus_test_service_new (NULL);

  hud_test_utils_json_loader_start_full (service, dbus_name, dbus_menu_path,
      "./test-indicator-source-session.json");
  GDBusConnection *connection = hud_test_utils_mock_dbus_connection_new (service,
      dbus_name, NULL);

  HudIndicatorSource* source = hud_indicator_source_new (connection);
  hud_test_utils_process_mainloop (100);

  g_assert(source != NULL);
  g_assert(HUD_IS_INDICATOR_SOURCE(source));

  HudTokenList *search = hud_token_list_new_from_string ("session time");

  {
    GPtrArray *results = g_ptr_array_new_with_free_func(g_object_unref);
    hud_source_search(HUD_SOURCE(source), search, hud_test_utils_results_append_func, results);
    g_assert_cmpuint(results->len, ==, 1);
    hud_test_utils_source_assert_result (results, 0, "Session Time");
    g_ptr_array_free(results, TRUE);
  }

  hud_token_list_free(search);
  g_object_unref (source);
  dbus_test_service_stop(service);
  g_object_unref (service);
  g_object_unref (connection);
}

static void
test_indicator_source_users ()
{
  const gchar *dbus_name = "com.canonical.indicator.session";
  const gchar *dbus_menu_path = "/com/canonical/indicator/users/menu";
//  const gchar *indicator_name = "indicator-session-user";

  DbusTestService *service = dbus_test_service_new (NULL);

  hud_test_utils_json_loader_start_full (service, dbus_name, dbus_menu_path,
      "./test-indicator-source-users.json");
  GDBusConnection *connection = hud_test_utils_mock_dbus_connection_new (service,
      dbus_name, NULL);

  HudIndicatorSource* source = hud_indicator_source_new (connection);
  hud_test_utils_process_mainloop (100);

  g_assert(source != NULL);
  g_assert(HUD_IS_INDICATOR_SOURCE(source));

  HudTokenList *search = hud_token_list_new_from_string ("users time");

  {
    GPtrArray *results = g_ptr_array_new_with_free_func(g_object_unref);
    hud_source_search(HUD_SOURCE(source), search, hud_test_utils_results_append_func, results);
    g_assert_cmpuint(results->len, ==, 1);
    hud_test_utils_source_assert_result (results, 0, "Users Time");
    g_ptr_array_free(results, TRUE);
  }

  hud_token_list_free(search);
  g_object_unref (source);
  dbus_test_service_stop(service);
  g_object_unref (service);
  g_object_unref (connection);
}

static void
test_indicator_source_sound ()
{
  const gchar *dbus_name = "com.canonical.indicator.sound";
  const gchar *dbus_menu_path = "/com/canonical/indicator/sound/menu";
//  const gchar *indicator_name = "indicator-sound";

  DbusTestService *service = dbus_test_service_new (NULL);

  hud_test_utils_json_loader_start_full (service, dbus_name, dbus_menu_path,
      "./test-indicator-source-sound.json");
  GDBusConnection *connection = hud_test_utils_mock_dbus_connection_new (service,
      dbus_name, NULL);

  HudIndicatorSource* source = hud_indicator_source_new (connection);
  hud_test_utils_process_mainloop (100);

  g_assert(source != NULL);
  g_assert(HUD_IS_INDICATOR_SOURCE(source));

  HudTokenList *search = hud_token_list_new_from_string ("sound time");

  {
    GPtrArray *results = g_ptr_array_new_with_free_func(g_object_unref);
    hud_source_search(HUD_SOURCE(source), search, hud_test_utils_results_append_func, results);
    g_assert_cmpuint(results->len, ==, 1);
    hud_test_utils_source_assert_result (results, 0, "Sound Time");
    g_ptr_array_free(results, TRUE);
  }

  hud_token_list_free(search);
  g_object_unref (source);
  dbus_test_service_stop(service);
  g_object_unref (service);
  g_object_unref (connection);
}

static void
test_indicator_source_messages ()
{
  const gchar *dbus_name = "com.canonical.indicator.messages";
  const gchar *dbus_menu_path = "/com/canonical/indicator/messages/menu";
//  const gchar *indicator_name = "indicator-messages";

  DbusTestService *service = dbus_test_service_new (NULL);

  hud_test_utils_start_menu_model_full(service, "./test-menu-input-model-simple", dbus_name, dbus_menu_path);
  GDBusConnection *connection = hud_test_utils_mock_dbus_connection_new (service,
      dbus_name, NULL);

  HudIndicatorSource* source = hud_indicator_source_new (connection);
  hud_test_utils_process_mainloop (100);

  g_assert(source != NULL);
  g_assert(HUD_IS_INDICATOR_SOURCE(source));

  HudTokenList *search = hud_token_list_new_from_string ("simple");

  {
    GPtrArray *results = g_ptr_array_new_with_free_func(g_object_unref);
    hud_source_search(HUD_SOURCE(source), search, hud_test_utils_results_append_func, results);
    g_assert_cmpuint(results->len, ==, 1);
    hud_test_utils_source_assert_result (results, 0, "Simple");
    g_ptr_array_free(results, TRUE);
  }

  hud_token_list_free(search);
  g_object_unref (source);
  dbus_test_service_stop(service);
  g_object_unref (service);
  g_object_unref (connection);
}

int
main (int argc, char **argv)
{
  g_type_init ();

  g_test_init (&argc, &argv, NULL);
  g_test_add_func ("/hud/hudindicatorsource/datetime", test_indicator_source_datetime);
  g_test_add_func ("/hud/hudindicatorsource/session", test_indicator_source_session);
  g_test_add_func ("/hud/hudindicatorsource/users", test_indicator_source_users);
  g_test_add_func ("/hud/hudindicatorsource/sound", test_indicator_source_sound);
  g_test_add_func ("/hud/hudindicatorsource/messages", test_indicator_source_messages);

  return g_test_run ();
}
