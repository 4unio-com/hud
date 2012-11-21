
#include <libdbustest/dbus-test.h>
#include <hud-client.h>

static void
test_query_create (void)
{
	DbusTestService * service = dbus_test_service_new(NULL);

	/* HUD Service */
	DbusTestProcess * huds = dbus_test_process_new(HUD_SERVICE);
	dbus_test_service_add_task(service, DBUS_TEST_TASK(huds));
	g_object_unref(huds);

	/* Dummy */
	DbusTestTask * dummy = dbus_test_task_new();
	dbus_test_task_set_wait_for(dummy, "com.canonical.hud");
	dbus_test_service_add_task(service, dummy);
	g_object_unref(dummy);

	/* Get HUD up and running and us on that bus */
	dbus_test_service_start_tasks(service);

	GDBusConnection * session = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, NULL);
	g_dbus_connection_set_exit_on_close(session, FALSE);

	/* Create a query */
	HudClientQuery * query = hud_client_query_new("test");

	g_assert(DEE_IS_MODEL(hud_client_query_get_results_model(query)));
	g_assert(DEE_IS_MODEL(hud_client_query_get_appstack_model(query)));

	g_assert(g_strcmp0("test", hud_client_query_get_query(query)) == 0);

	g_object_unref(query);
	g_object_unref(service);
	g_object_unref(session);

	return;
}

static void
test_suite (void)
{
	g_test_add_func ("/hud/client/query/create",   test_query_create);

	return;
}

int
main (int argc, char * argv[])
{
	g_type_init();
	g_test_init(&argc, &argv, NULL);

	/* Test Suites */
	test_suite();

	return g_test_run();
}
