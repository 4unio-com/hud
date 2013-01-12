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

static void
test_app_indicator_source_add_object (GDBusConnection *connection)
{
  GError *error;
  GVariantBuilder *builder;

  /* ssa{sv}a(ssss) path, interface, properties, methods */
  /* methods: name, in_sig, out_sig, code */
  builder = g_variant_builder_new (G_VARIANT_TYPE ("(ssa{sv}a(ssss))"));
  g_variant_builder_add (builder, "s", "/menu");
  g_variant_builder_add (builder, "s", "com.canonical.dbusmenu");
  g_variant_builder_open(builder, G_VARIANT_TYPE_VARDICT);
//  g_variant_builder_add (builder, "{sv}", "a", g_variant_new_string ("a"));
  g_variant_builder_close(builder);
  g_variant_builder_open(builder, G_VARIANT_TYPE ("a(ssss)"));
  g_variant_builder_add (builder, "(ssss)", "GetLayout", "iias", "u(ia{sv}av)", "ret = (dbus.UInt32(0), (dbus.Int32(0), [], []))");
  g_variant_builder_add (builder, "(ssss)", "GetGroupProperties", "aias", "a(ia{sv})", "ret = [(dbus.Int32(0), [])]");
  g_variant_builder_close(builder);

  error = NULL;
  g_dbus_connection_call_sync (connection, APP_INDICATOR_SERVICE_BUS_NAME,
      APP_INDICATOR_SERVICE_OBJECT_PATH, "org.freedesktop.DBus.Mock",
      "AddObject", g_variant_builder_end (builder), NULL,
      G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
  g_variant_builder_unref (builder);
  if (error)
  {
    g_warning("%s %s\n", "The request failed:", error->message);
    g_error_free (error);
  }
}

static void
test_app_indicator_source_add_method (GDBusConnection *connection,
    const gchar *bus_name, const gchar *path, const gchar *interface,
    const gchar *name, const gchar *in_sig, const gchar *out_sig,
    const gchar *code)
{
  GError *error;

  /* interface, name, in_sig, out_sig, code */
  error = NULL;
  g_dbus_connection_call_sync (connection, bus_name, path,
      "org.freedesktop.DBus.Mock", "AddMethod",
      g_variant_new ("(sssss)", interface, name, in_sig, out_sig, code), NULL,
      G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
  if (error)
  {
    g_warning("%s %s\n", "The request failed:", error->message);
    g_error_free (error);
  }
}

static gpointer
test_app_indicator_thread_stuff (gpointer user_data)
{
  TestHudAppIndicatorThreadData *thread_data = (TestHudAppIndicatorThreadData*) user_data;

  test_app_indicator_source_add_method (thread_data->connection,
        APP_INDICATOR_SERVICE_BUS_NAME, APP_INDICATOR_SERVICE_OBJECT_PATH,
        APP_INDICATOR_SERVICE_IFACE, "GetApplications", "", "a(sisossssss)",
        "ret = [('icon', dbus.Int32(0), 'com.canonical.indicator.application', '/menu', 'icon_theme_path', 'label', 'guide', 'icon_desc', 'id', 'title')]");
  /* icon, position, dbus_name, menu, icon_theme_path, label, guide, icon_desc, id, title */

  test_app_indicator_source_add_object(thread_data->connection);

  return FALSE;
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

  {
    TestHudAppIndicatorThreadData thread_data = {connection};
//    GThread* thread = g_thread_new ("update_query", test_app_indicator_thread_stuff, &thread_data);
//    g_thread_join(thread);
    test_app_indicator_thread_stuff(&thread_data);
  }

  HudAppIndicatorSource* source = hud_app_indicator_source_new (connection);
  hud_test_utils_process_mainloop (100);

  g_assert(source != NULL);
  g_assert(HUD_IS_APP_INDICATOR_SOURCE(source));

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
