
#include <libdbustest/dbus-test.h>
#include <hud-client.h>

#include "hudtestutils.h"

/* Kill everything if we fail */
static gboolean
unable_to_start_hud (gpointer user_data)
{
	g_error("Unable to start HUD service in time");
	return FALSE;
}

/* Pull all the code to start the HUD service into one helper function */
static void
start_hud_service (DbusTestService ** service, GDBusConnection ** session)
{
	*service = dbus_test_service_new(NULL);

	/* HUD Service */
	DbusTestProcess * huds = dbus_test_process_new(HUD_SERVICE);
	dbus_test_task_set_name(DBUS_TEST_TASK(huds), "HUD Service");
	dbus_test_service_add_task(*service, DBUS_TEST_TASK(huds));
	g_object_unref(huds);

	/* Dummy */
	DbusTestTask * dummy = dbus_test_task_new();
	dbus_test_task_set_wait_for(dummy, "com.canonical.hud");
	dbus_test_service_add_task(*service, dummy);
	g_object_unref(dummy);

	/* Add a timeout */
	gulong timeout = g_timeout_add_seconds(5, unable_to_start_hud, NULL);

	/* Get HUD up and running and us on that bus */
	g_debug("Starting up HUD service");
	dbus_test_service_start_tasks(*service);

	/* Remove timeout */
	g_source_remove(timeout);

	/* Set us not to exit when the service goes */
	*session = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, NULL);
	g_dbus_connection_set_exit_on_close(*session, FALSE);

	return;
}

static gboolean
no_dee_add_match (const gchar * log_domain, GLogLevelFlags level, const gchar * message, gpointer user_data)
{
	if (g_strcmp0(log_domain, "GLib-GIO") == 0 && 
			g_str_has_prefix(message, "Error while sending AddMatch()")) {
		return FALSE;
	}

	if (g_strcmp0(log_domain, "GLib-GIO") == 0 && 
			g_str_has_prefix(message, "g_dbus_connection_call_finish_internal: assertion 'G_IS_DBUS_CONNECTION'")) {
		return FALSE;
	}

	return TRUE;
}

static void
test_connection_create (void)
{
	DbusTestService * service = NULL;
	GDBusConnection * session = NULL;

	start_hud_service (&service, &session);

	/* Create a connection */
	HudClientConnection * con = hud_client_connection_new("com.canonical.hud", "/com/canonical/hud");

	g_assert(HUD_CLIENT_IS_CONNECTION(con));

	gchar * query_path = NULL;
	gchar * results_name = NULL;
	gchar * appstack_name = NULL;

	g_assert(hud_client_connection_new_query(con, "test", &query_path, &results_name, &appstack_name));
	g_assert(query_path != NULL);
	g_assert(results_name != NULL);
	g_assert(appstack_name != NULL);

	g_free(query_path);
	g_free(results_name);
	g_free(appstack_name);

	g_assert(g_strcmp0("com.canonical.hud", hud_client_connection_get_address(con)) == 0);

	gchar * address = NULL;
	gchar * path = NULL;

	g_object_get(G_OBJECT(con), "address", &address, "path", &path, NULL);

	g_assert(g_strcmp0(address, "com.canonical.hud") == 0);
	g_assert(g_strcmp0(path, "/com/canonical/hud") == 0);

	g_free(address);
	g_free(path);

	g_object_unref(con);
	g_object_unref(service);
	hud_test_utils_wait_for_connection_close(session);

	return;
}

static void
test_query_create (void)
{
	g_test_log_set_fatal_handler(no_dee_add_match, NULL);

	DbusTestService * service = NULL;
	GDBusConnection * session = NULL;

	start_hud_service (&service, &session);

	/* Create a query */
	HudClientQuery * query = hud_client_query_new("test");

	g_assert(DEE_IS_MODEL(hud_client_query_get_results_model(query)));
	g_assert(DEE_IS_MODEL(hud_client_query_get_appstack_model(query)));

	g_assert(g_strcmp0("test", hud_client_query_get_query(query)) == 0);

	HudClientConnection * con = NULL;
	gchar * search = NULL;

	g_object_get(G_OBJECT(query), "query", &search, "connection", &con, NULL);

	g_assert(g_strcmp0("test", search) == 0);
	g_assert(HUD_CLIENT_IS_CONNECTION(con));
	
	g_free(search);
	g_object_unref(con);

	g_object_unref(query);
	g_object_unref(service);
	hud_test_utils_wait_for_connection_close(session);

	return;
}

static void
test_query_update (void)
{
	g_test_log_set_fatal_handler(no_dee_add_match, NULL);
	DbusTestService * service = NULL;
	GDBusConnection * session = NULL;

	start_hud_service (&service, &session);

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
	g_object_unref(service);
	hud_test_utils_wait_for_connection_close(session);

	return;
}

static void
test_query_custom (void)
{
	g_test_log_set_fatal_handler(no_dee_add_match, NULL);

	DbusTestService * service = NULL;
	GDBusConnection * session = NULL;

	start_hud_service (&service, &session);

	/* Create a connection */
	HudClientConnection * con = hud_client_connection_new("com.canonical.hud", "/com/canonical/hud");
	g_assert(HUD_CLIENT_IS_CONNECTION(con));

	/* Create a query */
	HudClientQuery * query = hud_client_query_new_for_connection("test", con);
	g_assert(HUD_CLIENT_IS_QUERY(query));

	/* Make sure it has models */
	g_assert(DEE_IS_MODEL(hud_client_query_get_results_model(query)));
	g_assert(DEE_IS_MODEL(hud_client_query_get_appstack_model(query)));

	g_assert(g_strcmp0("test", hud_client_query_get_query(query)) == 0);

	/* Make sure the connection is the same */
	HudClientConnection * testcon = NULL;

	g_object_get(G_OBJECT(query), "connection", &testcon, NULL);

	g_assert(HUD_CLIENT_IS_CONNECTION(testcon));
	g_assert(testcon == con);
	g_object_unref(testcon);

	/* Clean up */
	g_object_unref(con);
	g_object_unref(query);
	g_object_unref(service);
	hud_test_utils_wait_for_connection_close(session);

	return;
}

static void
test_suite (void)
{
	g_test_add_func ("/hud/client/connection/create",   test_connection_create);
	g_test_add_func ("/hud/client/query/create",   test_query_create);
	g_test_add_func ("/hud/client/query/update",   test_query_update);
	g_test_add_func ("/hud/client/query/custom",   test_query_custom);

	return;
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
