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

typedef struct
{
  GDBusConnection *connection;
} TestHudAppIndicatorThreadData;

static const gchar * GET_LAYOUT = ""
    "if args[0] == 0 and args[1] == -1 and args[2] == ['type', 'label', 'visible', 'enabled', 'children-display', 'accessible-desc']:\n"
    "    empty = dbus.Array(signature='v')\n"
    "    root = dbus.Dictionary({'children-display': 'submenu', 'enabled': True, 'label': 'Label Empty', 'visible': True}, signature='sv')\n"
    "    sub_hello = dbus.Dictionary({'enabled': True, 'label': 'Hello There', 'visible': True}, signature='sv')\n"
    "    sub_goodbye = dbus.Dictionary({'enabled': True, 'label': 'Goodbye', 'visible': True}, signature='sv')\n"
    "    sub_hallo = dbus.Dictionary({'enabled': True, 'label': 'Hallo Again', 'visible': True}, signature='sv')\n"
    "    ret = (dbus.UInt32(2),\n"
    "      (dbus.Int32(0), root,\n"
    "        dbus.Array([\n"
    "          (dbus.Int32(1), sub_hello, empty),\n"
    "          (dbus.Int32(2), sub_goodbye, empty),\n"
    "          (dbus.Int32(3), sub_hallo, empty)\n"
    "        ], signature='v')\n"
    "      )\n"
    "    )\n"
    "else:\n"
    "    ret = Nil\n";

static const char* GET_GROUP_PROPERTIES = ""
    "results = [\n"
    "    (0, {'children-display': 'submenu'}),\n"
    "    (1, {'label': 'Hello There'}),\n"
    "    (2, {'label': 'Goodbye'}),\n"
    "    (3, {'label': 'Hallo Again'})\n"
    "]\n"
    "ret = dbus.Array(results, signature='(ia{sv})')";

static void
test_app_indicator_append_func(HudResult *result, gpointer user_data)
{
  g_assert(result != NULL);
  g_assert(HUD_IS_RESULT(result));

  HudItem * item = hud_result_get_item (result);
  g_assert(item != NULL);
  g_assert(HUD_IS_ITEM(item));

  g_assert(user_data != NULL);
  GPtrArray *results = (GPtrArray *) user_data;

  g_ptr_array_add(results, item);

  g_object_unref (result);
}

static void
test_app_indicator_source_new ()
{
  DbusTestService *service = NULL;
  GDBusConnection *connection = NULL;

  hud_test_utils_start_python_dbusmock (&service, &connection,
      APP_INDICATOR_SERVICE_BUS_NAME, APP_INDICATOR_SERVICE_OBJECT_PATH,
      APP_INDICATOR_SERVICE_IFACE);
  hud_test_utils_process_mainloop (100);

  dbus_mock_add_method (connection,
        APP_INDICATOR_SERVICE_BUS_NAME, APP_INDICATOR_SERVICE_OBJECT_PATH,
        APP_INDICATOR_SERVICE_IFACE, "GetApplications", "", "a(sisossssss)",
        "ret = [('icon', dbus.Int32(0), 'com.canonical.indicator.application', '/menu', 'icon_theme_path', 'label', 'guide', 'icon_desc', 'id', 'title')]");
  /* icon, position, dbus_name, menu, icon_theme_path, label, guide, icon_desc, id, title */

  GHashTable* properties = dbus_mock_new_properties ();
  DBusMockMethods* methods = dbus_mock_new_methods ();
  dbus_mock_methods_append(methods, "GetLayout", "iias", "u(ia{sv}av)", GET_LAYOUT);
  dbus_mock_methods_append (methods, "GetGroupProperties", "aias", "a(ia{sv})",
      GET_GROUP_PROPERTIES);
  dbus_mock_add_object (connection, APP_INDICATOR_SERVICE_BUS_NAME,
      APP_INDICATOR_SERVICE_OBJECT_PATH, "/menu", "com.canonical.dbusmenu",
      properties, methods);

  HudAppIndicatorSource* source = hud_app_indicator_source_new (connection);
  hud_test_utils_process_mainloop (100);

  g_assert(source != NULL);
  g_assert(HUD_IS_APP_INDICATOR_SOURCE(source));

  HudTokenList *search = hud_token_list_new_from_string ("hello");

  GPtrArray *results = g_ptr_array_new();
  hud_source_search(HUD_SOURCE(source), search, test_app_indicator_append_func, results);
  g_assert_cmpuint(results->len, ==, 2);

  {
    g_assert(HUD_IS_ITEM(g_ptr_array_index(results, 0)));
    HudItem *item = HUD_ITEM(g_ptr_array_index(results, 0));
    HudStringList * tokens = hud_item_get_tokens(item);
    g_assert_cmpstr(hud_string_list_get_head(tokens), ==, "Hello There");
  }

  {
    g_assert(HUD_IS_ITEM(g_ptr_array_index(results, 1)));
    HudItem *item = HUD_ITEM(g_ptr_array_index(results, 1));
    HudStringList * tokens = hud_item_get_tokens(item);
    g_assert_cmpstr(hud_string_list_get_head(tokens), ==, "Hallo Again");
  }

  hud_token_list_free(search);
  g_ptr_array_free(results, TRUE);
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
  g_test_add_func ("/hud/hudappindicatorsource/new", test_app_indicator_source_new);

  return g_test_run ();
}
