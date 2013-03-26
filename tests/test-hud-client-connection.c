
#include <libdbustest/dbus-test.h>
#include <hud-client.h>
#include "shared-values.h"

#include "hudtestutils.h"

static void
test_connection_create (void)
{
	DbusTestService *service = NULL;
	GDBusConnection *connection = NULL;
	DeeModel *results_model = NULL;
	DeeModel *appstack_model = NULL;

  hud_test_utils_start_hud_service (&service, &connection, &results_model,
      &appstack_model);

	/* Create a connection */
  HudClientConnection *client_connection = hud_client_connection_new (DBUS_NAME,
      DBUS_PATH);

  g_assert(HUD_CLIENT_IS_CONNECTION(client_connection));

  gchar *query_path = NULL;
  gchar *results_name = NULL;
  gchar *appstack_name = NULL;

  g_assert(
      hud_client_connection_new_query(client_connection, "test", &query_path, &results_name, &appstack_name));
  g_assert(query_path != NULL);
  g_assert(results_name != NULL);
  g_assert(appstack_name != NULL);

  g_free (query_path);
  g_free (results_name);
  g_free (appstack_name);

  g_assert_cmpstr(hud_client_connection_get_address(client_connection), ==,
      DBUS_NAME);

  gchar *address = NULL;
  gchar *path = NULL;

  g_object_get (G_OBJECT(client_connection), "address", &address, "path", &path,
      NULL );

  g_assert_cmpstr(address, ==, DBUS_NAME);
  g_assert_cmpstr(path, ==, DBUS_PATH);

  g_free (address);
  g_free (path);

  g_object_unref (client_connection);

  hud_test_utils_stop_hud_service (service, connection, results_model,
      appstack_model);
}

static void
test_suite (void)
{
	g_test_add_func ("/hud/client/connection/create", test_connection_create);
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
