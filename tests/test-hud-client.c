
#include <libdbustest/dbus-test.h>
#include <hud-client.h>
#include "shared-values.h"

#include "hudtestutils.h"

static gboolean
no_dee_add_match (const gchar * log_domain, GLogLevelFlags level, const gchar * message, gpointer user_data)
{
	if (g_strcmp0(log_domain, "GLib-GIO") == 0 && 
			g_str_has_prefix(message, "Error while sending AddMatch()")) {
		return FALSE;
	}

	if (g_strcmp0(log_domain, "GLib-GIO") == 0 && 
			g_str_has_prefix(message, "g_dbus_connection_call_finish_internal: assertion 'G_IS_DBUS_CONNECTION")) {
		return FALSE;
	}

	if (g_strcmp0(log_domain, "GLib-GIO") == 0 && 
			g_str_has_prefix(message, "g_dbus_connection_call_finish_internal: assertion `G_IS_DBUS_CONNECTION")) {
		return FALSE;
	}

	return TRUE;
}

static void
test_connection_create (void)
{
	DbusTestService *service = NULL;
	GDBusConnection *connection = NULL;
	HudQueryIfaceComCanonicalHudQuery *query_skel = NULL;
	DeeModel *results_model = NULL;
	DeeModel *appstack_model = NULL;

  hud_test_utils_start_hud_service (&service, &connection, &query_skel,
      &results_model, &appstack_model);

	/* Create a connection */
  HudClientConnection *client_connection = hud_client_connection_new (DBUS_NAME,
      DBUS_PATH);

	g_assert(HUD_CLIENT_IS_CONNECTION(client_connection));

	gchar * query_path = NULL;
	gchar * results_name = NULL;
	gchar * appstack_name = NULL;

	g_assert(hud_client_connection_new_query(client_connection, "test", &query_path, &results_name, &appstack_name));
	g_assert(query_path != NULL);
	g_assert(results_name != NULL);
	g_assert(appstack_name != NULL);

	g_free(query_path);
	g_free(results_name);
	g_free(appstack_name);

	g_assert(g_strcmp0(DBUS_NAME, hud_client_connection_get_address(client_connection)) == 0);

	gchar * address = NULL;
	gchar * path = NULL;

	g_object_get(G_OBJECT(client_connection), "address", &address, "path", &path, NULL);

	g_assert(g_strcmp0(address, DBUS_NAME) == 0);
	g_assert(g_strcmp0(path, DBUS_PATH) == 0);

	g_free(address);
	g_free(path);

	g_object_unref(client_connection);

  hud_test_utils_stop_hud_service (service, connection, query_skel,
      results_model, appstack_model);
}

static void
test_query_create (void)
{
	g_test_log_set_fatal_handler(no_dee_add_match, NULL);

	DbusTestService *service = NULL;
	GDBusConnection *connection = NULL;
	HudQueryIfaceComCanonicalHudQuery *query_skel = NULL;
	DeeModel *results_model = NULL;
  DeeModel *appstack_model = NULL;

  hud_test_utils_start_hud_service (&service, &connection, &query_skel,
      &results_model, &appstack_model);

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

  hud_test_utils_stop_hud_service (service, connection, query_skel,
      results_model, appstack_model);
}

static void
test_query_update (void)
{
	g_test_log_set_fatal_handler(no_dee_add_match, NULL);
	DbusTestService * service = NULL;
	GDBusConnection * connection = NULL;
	HudQueryIfaceComCanonicalHudQuery *query_skel = NULL;
	DeeModel *results_model = NULL;
  DeeModel *appstack_model = NULL;

  hud_test_utils_start_hud_service (&service, &connection, &query_skel,
      &results_model, &appstack_model);

	/* Create a query */
	g_print("Building a Query\n");
	HudClientQuery * query = hud_client_query_new("test");

	g_assert(g_strcmp0("test", hud_client_query_get_query(query)) == 0);

	g_print("Updating Query\n");
	hud_client_query_set_query(query, "test2");
	g_assert(g_strcmp0("test2", hud_client_query_get_query(query)) == 0);

	g_print("Setting a long Query\n");
	hud_client_query_set_query(query, "test a really really long query string that is probably too long to be resonable");
	g_assert(g_strcmp0("test a really really long query string that is probably too long to be resonable", hud_client_query_get_query(query)) == 0);

	g_object_unref(query);

  hud_test_utils_stop_hud_service (service, connection, query_skel,
      results_model, appstack_model);
}

static void
test_query_custom (void)
{
	g_test_log_set_fatal_handler(no_dee_add_match, NULL);

	DbusTestService *service = NULL;
	GDBusConnection *connection = NULL;
	HudQueryIfaceComCanonicalHudQuery *query_skel = NULL;
	DeeModel *results_model = NULL;
  DeeModel *appstack_model = NULL;

  hud_test_utils_start_hud_service (&service, &connection, &query_skel,
      &results_model, &appstack_model);

	/* Create a connection */
	HudClientConnection * client_connection = hud_client_connection_new(DBUS_NAME, DBUS_PATH);
	g_assert(HUD_CLIENT_IS_CONNECTION(client_connection));

	/* Create a query */
	HudClientQuery * query = hud_client_query_new_for_connection("test", client_connection);
	g_assert(HUD_CLIENT_IS_QUERY(query));

	/* Make sure it has models */
	g_assert(DEE_IS_MODEL(hud_client_query_get_results_model(query)));
	g_assert(DEE_IS_MODEL(hud_client_query_get_appstack_model(query)));

	g_assert(g_strcmp0("test", hud_client_query_get_query(query)) == 0);

	/* Make sure the connection is the same */
	HudClientConnection * testcon = NULL;

	g_object_get(G_OBJECT(query), "connection", &testcon, NULL);

	g_assert(HUD_CLIENT_IS_CONNECTION(testcon));
	g_assert(testcon == client_connection);
	g_object_unref(testcon);

	/* Clean up */
	g_object_unref(query);
	g_object_unref(client_connection);

  hud_test_utils_stop_hud_service (service, connection, query_skel,
      results_model, appstack_model);
}

static void
test_query_execute_command (void)
{
  g_test_log_set_fatal_handler(no_dee_add_match, NULL);

  DbusTestService *service = NULL;
  GDBusConnection *connection = NULL;
  HudQueryIfaceComCanonicalHudQuery *query_skel = NULL;
  DeeModel *results_model = NULL;
  DeeModel *appstack_model = NULL;

  hud_test_utils_start_hud_service (&service, &connection, &query_skel,
      &results_model, &appstack_model);

  HudClientQuery *query = hud_client_query_new("test");

  hud_client_query_execute_command (query,
      g_variant_new_variant (g_variant_new_uint64 (1)), 1234);

  g_object_unref(query);

  hud_test_utils_stop_hud_service (service, connection, query_skel,
      results_model, appstack_model);
}

static void
test_suite (void)
{
	g_test_add_func ("/hud/client/connection/create", test_connection_create);
  g_test_add_func ("/hud/client/query/create", test_query_create);
  g_test_add_func ("/hud/client/query/update", test_query_update);
  g_test_add_func ("/hud/client/query/custom", test_query_custom);
  g_test_add_func ("/hud/client/query/execute_command", test_query_execute_command);
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
