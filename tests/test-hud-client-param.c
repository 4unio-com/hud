
#include <libdbustest/dbus-test.h>
#include <hud-client.h>
#include "shared-values.h"

#include "test-utils.h"

static void
test_param_create (void)
{

  DbusTestService *service = dbus_test_service_new (NULL );
    hud_test_utils_dbus_mock_start (service, DBUS_NAME, DBUS_PATH, DBUS_IFACE);
  GDBusConnection *connection = hud_test_utils_mock_dbus_connection_new (
      service, DBUS_NAME, NULL );
  hud_test_utils_process_mainloop (300);

  HudClientParam* param = hud_client_param_new ("app.dbus.name", "base_action", "/action/path",
      "/model/path", 1);

  g_object_unref(param);
  g_object_unref(service);

  hud_test_utils_wait_for_connection_close(connection);
}

static void
test_param_get_actions (void)
{

  DbusTestService *service = dbus_test_service_new (NULL );
    hud_test_utils_dbus_mock_start (service, DBUS_NAME, DBUS_PATH, DBUS_IFACE);
  hud_test_utils_json_loader_start_full (service, "app.dbus.name", "/menu",
      JSON_SOURCE_ONE);
  GDBusConnection *connection = hud_test_utils_mock_dbus_connection_new (
      service, DBUS_NAME, "app.dbus.name", NULL );
    hud_test_utils_process_mainloop (300);

  HudClientParam* param = hud_client_param_new ("app.dbus.name", "base_action", "/action/path",
      "/model/path", 1);

  GActionGroup *action_group = hud_client_param_get_actions(param);
  g_assert(action_group);

  g_object_unref(param);
  g_object_unref(service);

  hud_test_utils_wait_for_connection_close(connection);
}

static void
test_suite (void)
{
  g_test_add_func ("/hud/client/param/create", test_param_create);
  g_test_add_func ("/hud/client/param/get_actions", test_param_get_actions);
}

int
main (int argc, char * argv[])
{
#ifndef GLIB_VERSION_2_36
	g_type_init ();
#endif

	g_test_init(&argc, &argv, NULL);

	/* Test Suites */
	test_suite();

	return g_test_run();
}
