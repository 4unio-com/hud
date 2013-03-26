
#include <libdbustest/dbus-test.h>
#include <hud-client.h>
#include "shared-values.h"

#include "hudtestutils.h"

static void
test_param_create (void)
{
	g_test_log_set_fatal_handler(no_dee_add_match, NULL);

	DbusTestService *service = NULL;
	GDBusConnection *connection = NULL;
	DeeModel *results_model = NULL;
  DeeModel *appstack_model = NULL;

  hud_test_utils_start_hud_service (&service, &connection, &results_model,
      &appstack_model);

	/* Create a query */
	HudClientQuery * query = hud_client_query_new("test");

	g_assert(DEE_IS_MODEL(hud_client_query_get_results_model(query)));
	g_assert(DEE_IS_MODEL(hud_client_query_get_appstack_model(query)));

	g_assert(g_strcmp0("test", hud_client_query_get_query(query)) == 0);

	HudClientConnection * client_connection = NULL;
	gchar * search = NULL;

	g_object_get(G_OBJECT(query), "query", &search, "connection", &client_connection, NULL);

	g_assert(g_strcmp0("test", search) == 0);
	g_assert(HUD_CLIENT_IS_CONNECTION(client_connection));
	
	g_free(search);

	g_object_unref(query);

	g_object_unref(client_connection);

  hud_test_utils_stop_hud_service (service, connection, results_model,
      appstack_model);
}

static void
test_suite (void)
{
  g_test_add_func ("/hud/client/param/create", test_param_create);
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
