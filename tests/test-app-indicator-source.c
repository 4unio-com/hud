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
#include "hudappindicatorsource.h"
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
test_app_indicator_source_add_remove ()
{
  DbusTestService *service = dbus_test_service_new (NULL);
  hud_test_utils_dbus_mock_start (service, APP_INDICATOR_SERVICE_BUS_NAME,
      APP_INDICATOR_SERVICE_OBJECT_PATH, APP_INDICATOR_SERVICE_IFACE);
  hud_test_utils_json_loader_start_full (service, "menu.one", "/menu/one",
      "./test-app-indicator-source-one.json");
  hud_test_utils_json_loader_start_full (service, "menu.two", "/menu/two",
        "./test-app-indicator-source-two.json");
  GDBusConnection *connection = hud_test_utils_mock_dbus_connection_new (service,
      APP_INDICATOR_SERVICE_BUS_NAME, "menu.one", "menu.two", NULL);
  hud_test_utils_process_mainloop (300);

  /* icon, position, dbus_name, menu, icon_theme_path, label, guide, icon_desc, id, title */
  dbus_mock_add_method (connection,
        APP_INDICATOR_SERVICE_BUS_NAME, APP_INDICATOR_SERVICE_OBJECT_PATH,
        APP_INDICATOR_SERVICE_IFACE, "GetApplications", "", "a(sisossssss)",
        "ret = [('icon.one', dbus.Int32(0), 'menu.one', '/menu/one', 'icon_theme_path.one', 'label.one', 'guide.one', 'icon_desc.one', 'id-one', 'title.one')]");
  hud_test_utils_process_mainloop (100);

  HudAppIndicatorSource* source = hud_app_indicator_source_new (connection);
  hud_test_utils_process_mainloop (100);

  g_assert(source != NULL);
  g_assert(HUD_IS_APP_INDICATOR_SOURCE(source));

  HudTokenList *search = hud_token_list_new_from_string ("hello");

  {
    GPtrArray *results = g_ptr_array_new_with_free_func(g_object_unref);
    hud_source_search(HUD_SOURCE(source), search, hud_test_utils_results_append_func, results);
    g_assert_cmpuint(results->len, ==, 2);
    g_ptr_array_sort(results, hud_test_utils_results_compare_func);
    hud_test_utils_source_assert_result (results, 0, "Hello There");
    hud_test_utils_source_assert_result (results, 1, "Hallo Again");
    g_ptr_array_free(results, TRUE);
  }

  {
    /* iconname, position, dbusaddress, dbusobject, iconpath, label, labelguide,
     * accessibledesc, hint, title */
    DBusMockSignalArgs* args = dbus_mock_new_signal_args ();
    dbus_mock_signal_args_append(args, g_variant_new("s", "iconname"));
    dbus_mock_signal_args_append(args, g_variant_new("i", 1));
    dbus_mock_signal_args_append(args, g_variant_new("s", "menu.two"));
    dbus_mock_signal_args_append(args, g_variant_new("o", "/menu/two"));
    dbus_mock_signal_args_append(args, g_variant_new("s", "iconpath.two"));
    dbus_mock_signal_args_append(args, g_variant_new("s", "label.two"));
    dbus_mock_signal_args_append(args, g_variant_new("s", "labelguide,two"));
    dbus_mock_signal_args_append(args, g_variant_new("s", "accessibledesc.two"));
    dbus_mock_signal_args_append(args, g_variant_new("s", "id-two"));
    dbus_mock_signal_args_append(args, g_variant_new("s", "title.two"));

    dbus_mock_emit_signal (connection, APP_INDICATOR_SERVICE_BUS_NAME,
        APP_INDICATOR_SERVICE_OBJECT_PATH, APP_INDICATOR_SERVICE_IFACE,
        "ApplicationAdded", "sisossssss", args);

    hud_test_utils_process_mainloop (100);

    GPtrArray *results = g_ptr_array_new_with_free_func(g_object_unref);
    hud_source_search(HUD_SOURCE(source), search, hud_test_utils_results_append_func, results);
    g_assert_cmpuint(results->len, ==, 4);
    g_ptr_array_sort(results, hud_test_utils_results_compare_func);
    hud_test_utils_source_assert_result (results, 0, "Hello There");
    hud_test_utils_source_assert_result (results, 1, "Hello There 2");
    hud_test_utils_source_assert_result (results, 2, "Hallo Again");
    hud_test_utils_source_assert_result (results, 3, "Hallo Again 2");
    g_ptr_array_free(results, TRUE);
  }

  {
    /* position */
    DBusMockSignalArgs* args = dbus_mock_new_signal_args ();
    dbus_mock_signal_args_append(args, g_variant_new("i", 0));

    dbus_mock_emit_signal (connection, APP_INDICATOR_SERVICE_BUS_NAME,
        APP_INDICATOR_SERVICE_OBJECT_PATH, APP_INDICATOR_SERVICE_IFACE,
        "ApplicationRemoved", "i", args);

    hud_test_utils_process_mainloop (100);

    GPtrArray *results = g_ptr_array_new_with_free_func(g_object_unref);
    hud_source_search(HUD_SOURCE(source), search, hud_test_utils_results_append_func, results);
    g_assert_cmpuint(results->len, ==, 2);
    g_ptr_array_sort(results, hud_test_utils_results_compare_func);
    hud_test_utils_source_assert_result (results, 0, "Hello There 2");
    hud_test_utils_source_assert_result (results, 1, "Hallo Again 2");
    g_ptr_array_free(results, TRUE);
  }

  hud_token_list_free(search);
  g_object_unref (source);

  hud_test_utils_process_mainloop (100);

  dbus_test_service_stop(service);
  g_object_unref (service);
  g_object_unref (connection);
}

static void
test_app_indicator_source_start_empty ()
{
  DbusTestService *service = dbus_test_service_new (NULL);
  hud_test_utils_dbus_mock_start (service, APP_INDICATOR_SERVICE_BUS_NAME,
      APP_INDICATOR_SERVICE_OBJECT_PATH, APP_INDICATOR_SERVICE_IFACE);
  hud_test_utils_json_loader_start_full (service, "menu.one", "/menu/one",
      "./test-app-indicator-source-one.json");
  GDBusConnection *connection = hud_test_utils_mock_dbus_connection_new (service,
      APP_INDICATOR_SERVICE_BUS_NAME, "menu.one", NULL);
  hud_test_utils_process_mainloop (300);

  /* icon, position, dbus_name, menu, icon_theme_path, label, guide, icon_desc, id, title */
  dbus_mock_add_method (connection,
        APP_INDICATOR_SERVICE_BUS_NAME, APP_INDICATOR_SERVICE_OBJECT_PATH,
        APP_INDICATOR_SERVICE_IFACE, "GetApplications", "", "a(sisossssss)",
        "ret = []");
  hud_test_utils_process_mainloop (100);

  HudAppIndicatorSource* source = hud_app_indicator_source_new (connection);
  hud_test_utils_process_mainloop (100);

  g_assert(source != NULL);
  g_assert(HUD_IS_APP_INDICATOR_SOURCE(source));

  HudTokenList *search = hud_token_list_new_from_string ("hello");

  {
    GPtrArray *results = g_ptr_array_new_with_free_func(g_object_unref);
    hud_source_search(HUD_SOURCE(source), search, hud_test_utils_results_append_func, results);
    g_assert_cmpuint(results->len, ==, 0);
    g_ptr_array_free(results, TRUE);
  }

  {
    /* iconname, position, dbusaddress, dbusobject, iconpath, label, labelguide,
     * accessibledesc, hint, title */
    DBusMockSignalArgs* args = dbus_mock_new_signal_args ();
    dbus_mock_signal_args_append(args, g_variant_new("s", "iconname"));
    dbus_mock_signal_args_append(args, g_variant_new("i", 1));
    dbus_mock_signal_args_append(args, g_variant_new("s", "menu.one"));
    dbus_mock_signal_args_append(args, g_variant_new("o", "/menu/one"));
    dbus_mock_signal_args_append(args, g_variant_new("s", "iconpath.one"));
    dbus_mock_signal_args_append(args, g_variant_new("s", "label.one"));
    dbus_mock_signal_args_append(args, g_variant_new("s", "labelguide.one"));
    dbus_mock_signal_args_append(args, g_variant_new("s", "accessibledesc.one"));
    dbus_mock_signal_args_append(args, g_variant_new("s", "id-one"));
    dbus_mock_signal_args_append(args, g_variant_new("s", "title.one"));

    dbus_mock_emit_signal (connection, APP_INDICATOR_SERVICE_BUS_NAME,
        APP_INDICATOR_SERVICE_OBJECT_PATH, APP_INDICATOR_SERVICE_IFACE,
        "ApplicationAdded", "sisossssss", args);

    hud_test_utils_process_mainloop (100);

    GPtrArray *results = g_ptr_array_new_with_free_func(g_object_unref);
    hud_source_search(HUD_SOURCE(source), search, hud_test_utils_results_append_func, results);
    g_assert_cmpuint(results->len, ==, 2);
    g_ptr_array_sort(results, hud_test_utils_results_compare_func);
    hud_test_utils_source_assert_result (results, 0, "Hello There");
    hud_test_utils_source_assert_result (results, 1, "Hallo Again");
    g_ptr_array_free(results, TRUE);
  }

  {
    /* position */
    DBusMockSignalArgs* args = dbus_mock_new_signal_args ();
    dbus_mock_signal_args_append(args, g_variant_new("i", 0));

    dbus_mock_emit_signal (connection, APP_INDICATOR_SERVICE_BUS_NAME,
        APP_INDICATOR_SERVICE_OBJECT_PATH, APP_INDICATOR_SERVICE_IFACE,
        "ApplicationRemoved", "i", args);

    hud_test_utils_process_mainloop (100);

    GPtrArray *results = g_ptr_array_new_with_free_func(g_object_unref);
    hud_source_search(HUD_SOURCE(source), search, hud_test_utils_results_append_func, results);
    g_assert_cmpuint(results->len, ==, 0);
    g_ptr_array_free(results, TRUE);
  }

  hud_token_list_free(search);
  g_object_unref (source);

  hud_test_utils_process_mainloop (100);

  dbus_test_service_stop(service);
  g_object_unref (service);
  g_object_unref (connection);
}

static void
test_app_indicator_source_change_title ()
{
  DbusTestService *service = dbus_test_service_new (NULL);
  hud_test_utils_dbus_mock_start (service, APP_INDICATOR_SERVICE_BUS_NAME,
      APP_INDICATOR_SERVICE_OBJECT_PATH, APP_INDICATOR_SERVICE_IFACE);
  hud_test_utils_json_loader_start_full (service, "menu.one", "/menu/one",
      "./test-app-indicator-source-one.json");
  GDBusConnection *connection = hud_test_utils_mock_dbus_connection_new (service,
      APP_INDICATOR_SERVICE_BUS_NAME, "menu.one", NULL);
  hud_test_utils_process_mainloop (300);

  /* icon, position, dbus_name, menu, icon_theme_path, label, guide, icon_desc, id, title */
  dbus_mock_add_method (connection,
        APP_INDICATOR_SERVICE_BUS_NAME, APP_INDICATOR_SERVICE_OBJECT_PATH,
        APP_INDICATOR_SERVICE_IFACE, "GetApplications", "", "a(sisossssss)",
        "ret = [('icon', dbus.Int32(0), 'menu.one', '/menu/one', 'icon_theme_path', 'label', 'guide', 'icon_desc', 'id-one', 'zzz')]");
  hud_test_utils_process_mainloop (100);

  HudAppIndicatorSource* source = hud_app_indicator_source_new (connection);
  hud_test_utils_process_mainloop (100);

  g_assert(source != NULL);
  g_assert(HUD_IS_APP_INDICATOR_SOURCE(source));

  HudTokenList *search_one = hud_token_list_new_from_string ("zzz");
  HudTokenList *search_two = hud_token_list_new_from_string ("aaaaa");

  {
    GPtrArray *results = g_ptr_array_new_with_free_func(g_object_unref);
    hud_source_search(HUD_SOURCE(source), search_one, hud_test_utils_results_append_func, results);
    g_assert_cmpuint(results->len, ==, 3);
    g_ptr_array_free(results, TRUE);
  }

  {
    GPtrArray *results = g_ptr_array_new_with_free_func(g_object_unref);
    hud_source_search(HUD_SOURCE(source), search_two, hud_test_utils_results_append_func, results);
    g_assert_cmpuint(results->len, ==, 0);
    g_ptr_array_free(results, TRUE);
  }

  {
    /* position,  title */
    DBusMockSignalArgs* args = dbus_mock_new_signal_args ();
    dbus_mock_signal_args_append(args, g_variant_new("i", 0));
    dbus_mock_signal_args_append(args, g_variant_new("s", "aaaaa"));

    dbus_mock_emit_signal (connection, APP_INDICATOR_SERVICE_BUS_NAME,
        APP_INDICATOR_SERVICE_OBJECT_PATH, APP_INDICATOR_SERVICE_IFACE,
        "ApplicationTitleChanged", "is", args);

    hud_test_utils_process_mainloop (100);
  }

  {
    GPtrArray *results = g_ptr_array_new_with_free_func(g_object_unref);
    hud_source_search(HUD_SOURCE(source), search_one, hud_test_utils_results_append_func, results);
    g_assert_cmpuint(results->len, ==, 0);
    g_ptr_array_free(results, TRUE);
  }

  {
    GPtrArray *results = g_ptr_array_new_with_free_func(g_object_unref);
    hud_source_search(HUD_SOURCE(source), search_two, hud_test_utils_results_append_func, results);
    g_assert_cmpuint(results->len, ==, 3);
    g_ptr_array_free(results, TRUE);
  }

  hud_token_list_free(search_one);
  hud_token_list_free(search_two);
  g_object_unref (source);

  hud_test_utils_process_mainloop (100);

  dbus_test_service_stop(service);
  g_object_unref (service);
  g_object_unref (connection);
}

int
main (int argc, char **argv)
{
#ifndef GLIB_VERSION_2_36
  g_type_init ();
#endif

  g_test_init (&argc, &argv, NULL);
  g_test_add_func ("/hud/hudappindicatorsource/add_remove", test_app_indicator_source_add_remove);
  g_test_add_func ("/hud/hudappindicatorsource/start_empty", test_app_indicator_source_start_empty);
  g_test_add_func ("/hud/hudappindicatorsource/change_title", test_app_indicator_source_change_title);

  return g_test_run ();
}
