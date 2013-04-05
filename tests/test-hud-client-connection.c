
#include <libdbustest/dbus-test.h>
#include <hud-client.h>
#include "shared-values.h"

#include "hudtestutils.h"

typedef struct _test_connection_create_t test_connection_create_t;
struct _test_connection_create_t {
	gchar * query_path;
	gchar * results_name;
	gchar * appstack_name;
	GMainLoop * loop;
};

static void
test_connection_create_query (HudClientConnection * con, const gchar * query_path, const gchar * results_name, const gchar * appstack_name, gpointer user_data)
{
	test_connection_create_t * data = (test_connection_create_t *)user_data;
	data->query_path = g_strdup(query_path);
	data->results_name = g_strdup(results_name);
	data->appstack_name = g_strdup(appstack_name);
	g_main_loop_quit(data->loop);
	return;
}

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

  test_connection_create_t data;
  data.query_path = NULL;
  data.results_name = NULL;
  data.appstack_name = NULL;
  data.loop = g_main_loop_new(NULL, FALSE);

  hud_client_connection_new_query(client_connection, "test", test_connection_create_query, &data);

  gulong sig = g_timeout_add_seconds(5, (GSourceFunc)g_main_loop_quit, data.loop);

  g_main_loop_run(data.loop);
  g_main_loop_unref(data.loop);

  g_source_remove(sig);

  g_assert(data.query_path != NULL);
  g_assert(data.results_name != NULL);
  g_assert(data.appstack_name != NULL);

  g_free(data.query_path);
  g_free(data.results_name);
  g_free(data.appstack_name);

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
