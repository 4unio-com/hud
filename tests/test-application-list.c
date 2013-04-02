
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
#include "application-list.h"
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

static const gchar *BAMF_BUS_NAME = "org.ayatana.bamf";
static const gchar *MATCHER_OBJECT_PATH = "/org/ayatana/bamf/matcher";
static const gchar *APPLICATION_INTERFACE_NAME = "org.ayatana.bamf.application";
static const gchar *MATCHER_INTERFACE_NAME = "org.ayatana.bamf.matcher";
static const gchar *VIEW_INTERFACE_NAME = "org.ayatana.bamf.view";
static const gchar *WINDOW_INTERFACE_NAME = "org.ayatana.bamf.window";
static const gchar* REGISTRAR_BUS_NAME = "com.canonical.AppMenu.Registrar";
static const gchar* REGISTRAR_OBJECT_PATH = "/com/canonical/AppMenu/Registrar";
static const gchar* REGISTRAR_INTERFACE_NAME = "com.canonical.AppMenu.Registrar";

static void
test_window_source_add_view_methods (GDBusConnection* connection,
    const gchar *object_path)
{
  dbus_mock_add_method (connection, BAMF_BUS_NAME, object_path,
      VIEW_INTERFACE_NAME, "Name", "", "s", "ret = 'name'");
  dbus_mock_add_method (connection, BAMF_BUS_NAME, object_path,
      VIEW_INTERFACE_NAME, "Children", "", "as", "ret = []");
  dbus_mock_add_method (connection, BAMF_BUS_NAME, object_path,
      VIEW_INTERFACE_NAME, "Icon", "", "s", "ret = 'icon.png'");
}

static void
test_window_source_menu_model ()
{
  const gchar *app_dbus_name = "app.dbus.name";
  const gchar *app_dbus_menu_path = "/app/dbus/menu/path";

  DbusTestService *service = dbus_test_service_new (NULL);
  hud_test_utils_dbus_mock_start (service, BAMF_BUS_NAME,
      MATCHER_OBJECT_PATH, MATCHER_INTERFACE_NAME);
  hud_test_utils_dbus_mock_start (service, REGISTRAR_BUS_NAME,
      REGISTRAR_OBJECT_PATH, REGISTRAR_INTERFACE_NAME);
  hud_test_utils_start_menu_model_full (service,
      MODEL_SIMPLE, app_dbus_name, app_dbus_menu_path,
      TRUE);
  hud_test_utils_json_loader_start_full(service, "app.dbus.name.two", "/menu", JSON_SHORTCUTS);
  GDBusConnection *connection = hud_test_utils_mock_dbus_connection_new (service,
      BAMF_BUS_NAME, REGISTRAR_BUS_NAME, app_dbus_name, "app.dbus.name.two", NULL );
  hud_test_utils_process_mainloop (300);

  /* Define the mock window */
  {
    DBusMockProperties* properties = dbus_mock_new_properties ();
    DBusMockMethods* methods = dbus_mock_new_methods ();
    dbus_mock_methods_append (methods, "GetXid", "", "u", "ret = 1");
    dbus_mock_methods_append (methods, "Monitor", "", "i", "ret = 0");
    dbus_mock_methods_append (methods, "Maximized", "", "i", "ret = 1");
    dbus_mock_methods_append (methods, "Xprop", "s", "s", ""
        "dict = {'_GTK_UNIQUE_BUS_NAME': 'app.dbus.name',\n"
        "       '_GTK_APP_MENU_OBJECT_PATH': '/app/dbus/menu/path',\n"
        "       '_GTK_MENUBAR_OBJECT_PATH': '',\n"
        "       '_GTK_APPLICATION_OBJECT_PATH': '/app/dbus/menu/path',\n"
        "       '_GTK_WINDOW_OBJECT_PATH': ''\n"
        "       }\n"
        "ret = dict[args[0]]");
    dbus_mock_add_object (connection, BAMF_BUS_NAME, MATCHER_OBJECT_PATH,
        "/org/ayatana/bamf/window00000001", WINDOW_INTERFACE_NAME, properties,
        methods);
    test_window_source_add_view_methods (connection, "/org/ayatana/bamf/window00000001");
  }

  /* Define the mock application */
  {
      DBusMockProperties* properties = dbus_mock_new_properties ();
      DBusMockMethods* methods = dbus_mock_new_methods ();
      dbus_mock_methods_append (methods, "DesktopFile", "", "s",
          "ret = '/usr/share/applications/name.desktop'");
      dbus_mock_add_object (connection, BAMF_BUS_NAME, MATCHER_OBJECT_PATH,
          "/org/ayatana/bamf/application00000001", APPLICATION_INTERFACE_NAME,
          properties, methods);
      test_window_source_add_view_methods (connection, "/org/ayatana/bamf/application00000001");
    }

  /* Set up the BAMF matcher */
  {
    dbus_mock_add_method (connection,
        BAMF_BUS_NAME, MATCHER_OBJECT_PATH,
        MATCHER_INTERFACE_NAME, "ActiveWindow", "", "s",
          "ret = '/org/ayatana/bamf/window00000001'");
    dbus_mock_add_method (connection,
        BAMF_BUS_NAME, MATCHER_OBJECT_PATH,
        MATCHER_INTERFACE_NAME, "ApplicationForXid", "u", "s",
          "if args[0] == 1:\n"
          "    ret = '/org/ayatana/bamf/application00000001'\n"
          "else:\n"
          "    ret = None");
    dbus_mock_add_method (connection,
        BAMF_BUS_NAME, MATCHER_OBJECT_PATH,
        MATCHER_INTERFACE_NAME, "ApplicationPaths", "", "as",
          "ret = ['/org/ayatana/bamf/application00000001']");
    dbus_mock_add_method (connection,
        BAMF_BUS_NAME, MATCHER_OBJECT_PATH,
        MATCHER_INTERFACE_NAME, "WindowPaths", "", "as",
          "ret = ['/org/ayatana/bamf/window00000001']");
    dbus_mock_add_method (connection,
        BAMF_BUS_NAME, MATCHER_OBJECT_PATH,
        MATCHER_INTERFACE_NAME, "ActiveApplication", "", "s",
          "ret = '/org/ayatana/bamf/application00000001'");
  }

  /* Set up the app registrar */
  {
    dbus_mock_add_method (connection,
        REGISTRAR_BUS_NAME, REGISTRAR_OBJECT_PATH,
        REGISTRAR_INTERFACE_NAME, "GetMenus", "", "a(uso)",
          "ret = [(1, 'app.dbus.name.two', '/menu')]");
  }

  hud_test_utils_process_mainloop (100);

  HudApplicationList* source = hud_application_list_new();
  g_assert(source != NULL);
  g_assert(HUD_IS_APPLICATION_LIST(source));

  hud_test_utils_process_mainloop (100);

  {
    /* old, new */
    DBusMockSignalArgs* args = dbus_mock_new_signal_args ();
    dbus_mock_signal_args_append(args, g_variant_new("s", ""));
    dbus_mock_signal_args_append(args, g_variant_new("s", "/org/ayatana/bamf/window00000001"));
    dbus_mock_emit_signal (connection, BAMF_BUS_NAME, MATCHER_OBJECT_PATH,
        MATCHER_INTERFACE_NAME, "ActiveWindowChanged", "ss", args);
  }

  hud_test_utils_process_mainloop (100);

  hud_source_use(HUD_SOURCE(source));

  HudTokenList *search = hud_token_list_new_from_string ("simple");
  {
    GPtrArray *results = g_ptr_array_new_with_free_func(g_object_unref);
    hud_source_search(HUD_SOURCE(source), search, hud_test_utils_results_append_func, results);
    g_assert_cmpuint(results->len, ==, 1);
    hud_test_utils_source_assert_result (results, 0, "Simple");
    g_ptr_array_free(results, TRUE);
  }

  HudTokenList *search_two = hud_token_list_new_from_string ("save");
  {
    GPtrArray *results = g_ptr_array_new_with_free_func(g_object_unref);
    hud_source_search(HUD_SOURCE(source), search_two, hud_test_utils_results_append_func, results);
    g_assert_cmpuint(results->len, ==, 1);
    hud_test_utils_source_assert_result (results, 0, "Save");
    g_ptr_array_free(results, TRUE);
  }

  {
    GList* apps = hud_application_list_get_apps (source);
    g_assert_cmpuint(g_list_length(apps), ==, 1);
    HudApplicationSource * app =
        HUD_APPLICATION_SOURCE(g_list_nth_data(apps, 0));
    g_assert(HUD_IS_APPLICATION_SOURCE(app));
    g_assert_cmpstr(hud_application_source_get_id (app), ==, "name");
    g_assert_cmpstr(hud_application_source_get_path (app), ==,
        "/com/canonical/hud/applications/name");
    g_list_free(apps);
  }

  {
    HudApplicationSource * app = hud_application_list_get_source (source,
        "name");
    g_assert(HUD_IS_APPLICATION_SOURCE(app));
    g_assert_cmpstr(hud_application_source_get_id (app), ==, "name");
    g_assert_cmpstr(hud_application_source_get_path (app), ==,
        "/com/canonical/hud/applications/name");
  }

  hud_source_unuse(HUD_SOURCE(source));

  hud_token_list_free(search);
  hud_token_list_free(search_two);
  g_object_unref (source);

  hud_test_utils_process_mainloop (100);

  g_object_unref (service);
  g_object_unref (connection);
  /* FIXME: We would like to do this here, but dbus makes us exit with a non-zero code
  hud_test_utils_wait_for_connection_close(connection);
  */
}

int
main (int argc, char **argv)
{
#ifndef GLIB_VERSION_2_36
  g_type_init ();
#endif

  g_test_init (&argc, &argv, NULL);
  g_test_add_func ("/hud/windowsource/menu_model", test_window_source_menu_model);

  return g_test_run ();
}
