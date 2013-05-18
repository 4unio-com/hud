
#include <libdbustest/dbus-test.h>
#include <hud-client.h>
#include "shared-values.h"

#include "hudtestutils.h"

/* Define the global default timeout for g_timeout_add_seconds */
#ifndef TEST_DEFAULT_TIMEOUT_S
#define TEST_DEFAULT_TIMEOUT_S 15
#endif


static gboolean
fail_quit (gpointer pmain)
{
	g_error("Timeout, mainloop took too long!");
	g_main_loop_quit((GMainLoop *)pmain);
	return FALSE;
}

static void
test_query_create_models_ready (HudClientQuery * query, gpointer user_data)
{
	g_main_loop_quit((GMainLoop *)user_data);
	return;
}

static void
test_query_create (void)
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

	/* Wait for the models to be ready */
	GMainLoop * loop = g_main_loop_new(NULL, FALSE);
	gulong sig = g_timeout_add_seconds(TEST_DEFAULT_TIMEOUT_S, fail_quit, loop);

	g_signal_connect(G_OBJECT(query), HUD_CLIENT_QUERY_SIGNAL_MODELS_CHANGED, G_CALLBACK(test_query_create_models_ready), loop);

	g_main_loop_run(loop);
	g_main_loop_unref(loop);

	g_source_remove(sig);

	/* Check the models */
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
test_query_custom (void)
{
	g_test_log_set_fatal_handler(no_dee_add_match, NULL);

	DbusTestService *service = NULL;
	GDBusConnection *connection = NULL;
	DeeModel *results_model = NULL;
  DeeModel *appstack_model = NULL;

  hud_test_utils_start_hud_service (&service, &connection, &results_model,
      &appstack_model);

	/* Create a connection */
  HudClientConnection * client_connection = hud_client_connection_new (
      DBUS_NAME, DBUS_PATH);
	g_assert(HUD_CLIENT_IS_CONNECTION(client_connection));

	/* Create a query */
	HudClientQuery * query = hud_client_query_new_for_connection("test", client_connection);
	g_assert(HUD_CLIENT_IS_QUERY(query));

	/* Wait for the models to be ready */
	GMainLoop * loop = g_main_loop_new(NULL, FALSE);
	gulong sig = g_timeout_add_seconds(TEST_DEFAULT_TIMEOUT_S, fail_quit, loop);

	g_signal_connect(G_OBJECT(query), HUD_CLIENT_QUERY_SIGNAL_MODELS_CHANGED, G_CALLBACK(test_query_create_models_ready), loop);

	g_main_loop_run(loop);
	g_main_loop_unref(loop);

	g_source_remove(sig);

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

  hud_test_utils_stop_hud_service (service, connection, results_model,
      appstack_model);
}

static void
test_query_update (void)
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
  g_assert(HUD_CLIENT_IS_QUERY(query));

  /* Wait for the models to be ready */
  GMainLoop * loop = g_main_loop_new(NULL, FALSE);
  gulong sig = g_timeout_add_seconds(TEST_DEFAULT_TIMEOUT_S, fail_quit, loop);

  g_signal_connect(G_OBJECT(query), HUD_CLIENT_QUERY_SIGNAL_MODELS_CHANGED, G_CALLBACK(test_query_create_models_ready), loop);

  g_main_loop_run(loop);
  g_main_loop_unref(loop);

  g_source_remove(sig);

  hud_client_query_set_query(query, "test2");
  g_assert_cmpstr(hud_client_query_get_query(query), ==, "test2");

  dbus_mock_assert_method_call_results (connection, DBUS_NAME, QUERY_PATH,
      "UpdateQuery", "\\(\\[\\(\\d+, \\[<'test2'>\\]\\)\\],\\)");

  g_object_unref(query);

  hud_test_utils_stop_hud_service (service, connection, results_model,
      appstack_model);
}

static gboolean
handle_voice_query_finished (HudClientQuery *query,
    GDBusMethodInvocation *invocation, gpointer user_data)
{
  gboolean *called = (gboolean *) user_data;
  *called = TRUE;
  return TRUE;
}

static void
test_query_voice (void)
{
  g_test_log_set_fatal_handler(no_dee_add_match, NULL);

  DbusTestService *service = NULL;
  GDBusConnection *connection = NULL;
  DeeModel *results_model = NULL;
  DeeModel *appstack_model = NULL;

  hud_test_utils_start_hud_service (&service, &connection, &results_model,
      &appstack_model);

  /* Create a query */
  HudClientQuery *query = hud_client_query_new("test");
  g_assert(HUD_CLIENT_IS_QUERY(query));

  /* Wait for the models to be ready */
  GMainLoop * loop = g_main_loop_new(NULL, FALSE);
  gulong sig = g_timeout_add_seconds(TEST_DEFAULT_TIMEOUT_S, fail_quit, loop);

  g_signal_connect(G_OBJECT(query), HUD_CLIENT_QUERY_SIGNAL_MODELS_CHANGED, G_CALLBACK(test_query_create_models_ready), loop);

  g_main_loop_run(loop);
  g_main_loop_unref(loop);

  g_source_remove(sig);

  /* Call the voice query */
  gboolean called = FALSE;
  g_signal_connect(G_OBJECT(query), "voice-query-finished",
      G_CALLBACK(handle_voice_query_finished), &called);

  hud_client_query_voice_query(query);
  hud_test_utils_process_mainloop (100);

  dbus_mock_assert_method_call_results (connection, DBUS_NAME, QUERY_PATH,
      "VoiceQuery", "\\(\\[\\(\\d+, \\[\\]\\)\\],\\)");

  g_assert(called);

  g_object_unref(query);

  hud_test_utils_stop_hud_service (service, connection, results_model,
      appstack_model);
}

static void
test_query_update_app (void)
{
  g_test_log_set_fatal_handler(no_dee_add_match, NULL);

  DbusTestService *service = NULL;
  GDBusConnection *connection = NULL;
  DeeModel *results_model = NULL;
  DeeModel *appstack_model = NULL;

  hud_test_utils_start_hud_service (&service, &connection, &results_model,
      &appstack_model);

  /* Create a query */
  HudClientQuery *query = hud_client_query_new("test");
  g_assert(HUD_CLIENT_IS_QUERY(query));

  /* Wait for the models to be ready */
  GMainLoop * loop = g_main_loop_new(NULL, FALSE);
  gulong sig = g_timeout_add_seconds(TEST_DEFAULT_TIMEOUT_S, fail_quit, loop);

  g_signal_connect(G_OBJECT(query), HUD_CLIENT_QUERY_SIGNAL_MODELS_CHANGED, G_CALLBACK(test_query_create_models_ready), loop);

  g_main_loop_run(loop);
  g_main_loop_unref(loop);

  g_source_remove(sig);

  /* Set App ID */
  hud_client_query_set_appstack_app(query, "application-id");

  dbus_mock_assert_method_call_results (connection, DBUS_NAME, QUERY_PATH,
      "UpdateApp", "\\(\\[\\(\\d+, \\[<'application-id'>\\]\\)\\],\\)");

  g_object_unref(query);

  hud_test_utils_stop_hud_service (service, connection, results_model,
      appstack_model);
}

static void
test_query_execute_command (void)
{
  g_test_log_set_fatal_handler(no_dee_add_match, NULL);

  DbusTestService *service = NULL;
  GDBusConnection *connection = NULL;
  DeeModel *results_model = NULL;
  DeeModel *appstack_model = NULL;

  hud_test_utils_start_hud_service (&service, &connection, &results_model,
      &appstack_model);

  /* Create a query */
  HudClientQuery *query = hud_client_query_new("test");
  g_assert(HUD_CLIENT_IS_QUERY(query));

  /* Wait for the models to be ready */
  GMainLoop * loop = g_main_loop_new(NULL, FALSE);
  gulong sig = g_timeout_add_seconds(TEST_DEFAULT_TIMEOUT_S, fail_quit, loop);

  g_signal_connect(G_OBJECT(query), HUD_CLIENT_QUERY_SIGNAL_MODELS_CHANGED, G_CALLBACK(test_query_create_models_ready), loop);

  g_main_loop_run(loop);
  g_main_loop_unref(loop);

  g_source_remove(sig);

  /* Execute a command */
  hud_client_query_execute_command (query,
      g_variant_new_variant (g_variant_new_uint64 (4321)), 1234);

  dbus_mock_assert_method_call_results (connection, DBUS_NAME, QUERY_PATH,
      "ExecuteCommand", "\\(\\[\\(\\d+, \\[<uint64 4321>, <uint32 1234>\\]\\)\\],\\)");

  g_object_unref(query);

  hud_test_utils_stop_hud_service (service, connection, results_model,
      appstack_model);
}

static void
test_query_execute_parameterized (void)
{
  g_test_log_set_fatal_handler(no_dee_add_match, NULL);

  DbusTestService *service = NULL;
  GDBusConnection *connection = NULL;
  DeeModel *results_model = NULL;
  DeeModel *appstack_model = NULL;

  hud_test_utils_start_hud_service (&service, &connection, &results_model,
      &appstack_model);

  /* Create a query */
  HudClientQuery *query = hud_client_query_new("test");
  g_assert(HUD_CLIENT_IS_QUERY(query));

  /* Wait for the models to be ready */
  GMainLoop * loop = g_main_loop_new(NULL, FALSE);
  gulong sig = g_timeout_add_seconds(TEST_DEFAULT_TIMEOUT_S, fail_quit, loop);

  g_signal_connect(G_OBJECT(query), HUD_CLIENT_QUERY_SIGNAL_MODELS_CHANGED, G_CALLBACK(test_query_create_models_ready), loop);

  g_main_loop_run(loop);
  g_main_loop_unref(loop);

  g_source_remove(sig);

  /* Execute a parameterized command */
  HudClientParam *param = hud_client_query_execute_param_command (query,
      g_variant_new_variant (g_variant_new_uint64 (4321)), 1234);

  dbus_mock_assert_method_call_results (connection, DBUS_NAME, QUERY_PATH,
      "ExecuteParameterized", "\\(\\[\\(\\d+, \\[<uint64 4321>, <uint32 1234>\\]\\)\\],\\)");

  g_object_unref(param);
  g_object_unref(query);

  hud_test_utils_stop_hud_service (service, connection, results_model,
      appstack_model);
}

static void
test_query_execute_toolbar (void)
{
  g_test_log_set_fatal_handler(no_dee_add_match, NULL);

  DbusTestService *service = NULL;
  GDBusConnection *connection = NULL;
  DeeModel *results_model = NULL;
  DeeModel *appstack_model = NULL;

  hud_test_utils_start_hud_service (&service, &connection, &results_model,
      &appstack_model);

  /* Create a query */
  HudClientQuery *query = hud_client_query_new("test");
  g_assert(HUD_CLIENT_IS_QUERY(query));

  /* Wait for the models to be ready */
  GMainLoop * loop = g_main_loop_new(NULL, FALSE);
  gulong sig = g_timeout_add_seconds(TEST_DEFAULT_TIMEOUT_S, fail_quit, loop);

  g_signal_connect(G_OBJECT(query), HUD_CLIENT_QUERY_SIGNAL_MODELS_CHANGED, G_CALLBACK(test_query_create_models_ready), loop);

  g_source_remove(sig);

  g_main_loop_run(loop);
  g_main_loop_unref(loop);

  /* Start attacking the toolbar */
  hud_client_query_execute_toolbar_item (query,
      HUD_CLIENT_QUERY_TOOLBAR_FULLSCREEN, 12345);
  dbus_mock_assert_method_call_results (connection, DBUS_NAME, QUERY_PATH,
      "ExecuteToolbar",
      "\\(\\[\\(\\d+, \\[<'fullscreen'>, <uint32 12345>\\]\\)\\],\\)");
  dbus_mock_clear_method_calls (connection, DBUS_NAME, QUERY_PATH);

  hud_client_query_execute_toolbar_item (query,
      HUD_CLIENT_QUERY_TOOLBAR_HELP, 12);
  dbus_mock_assert_method_call_results (connection, DBUS_NAME, QUERY_PATH,
      "ExecuteToolbar",
      "\\(\\[\\(\\d+, \\[<'help'>, <uint32 12>\\]\\)\\],\\)");
  dbus_mock_clear_method_calls (connection, DBUS_NAME, QUERY_PATH);

  hud_client_query_execute_toolbar_item (query,
      HUD_CLIENT_QUERY_TOOLBAR_PREFERENCES, 312);
  dbus_mock_assert_method_call_results (connection, DBUS_NAME, QUERY_PATH,
      "ExecuteToolbar",
      "\\(\\[\\(\\d+, \\[<'preferences'>, <uint32 312>\\]\\)\\],\\)");
  dbus_mock_clear_method_calls (connection, DBUS_NAME, QUERY_PATH);

  hud_client_query_execute_toolbar_item (query,
      HUD_CLIENT_QUERY_TOOLBAR_UNDO, 53312);
  dbus_mock_assert_method_call_results (connection, DBUS_NAME, QUERY_PATH,
      "ExecuteToolbar",
      "\\(\\[\\(\\d+, \\[<'undo'>, <uint32 53312>\\]\\)\\],\\)");
  dbus_mock_clear_method_calls (connection, DBUS_NAME, QUERY_PATH);

  g_object_unref(query);

  hud_test_utils_stop_hud_service (service, connection, results_model,
      appstack_model);
}

static void
test_query_toolbar_enabled (void)
{
  g_test_log_set_fatal_handler(no_dee_add_match, NULL);

  DbusTestService *service = NULL;
  GDBusConnection *connection = NULL;
  DeeModel *results_model = NULL;
  DeeModel *appstack_model = NULL;

  hud_test_utils_start_hud_service (&service, &connection, &results_model,
      &appstack_model);

  /* Create a query */
  HudClientQuery *query = hud_client_query_new("test");
  g_assert(HUD_CLIENT_IS_QUERY(query));

  /* Wait for the models to be ready */
  GMainLoop * loop = g_main_loop_new(NULL, FALSE);
  gulong sig = g_timeout_add_seconds(TEST_DEFAULT_TIMEOUT_S, fail_quit, loop);

  g_signal_connect(G_OBJECT(query), HUD_CLIENT_QUERY_SIGNAL_MODELS_CHANGED, G_CALLBACK(test_query_create_models_ready), loop);

  g_source_remove(sig);

  g_main_loop_run(loop);
  g_main_loop_unref(loop);

  /* Test toolbar disabled */
  g_assert(!hud_client_query_toolbar_item_active(query, HUD_CLIENT_QUERY_TOOLBAR_FULLSCREEN));
  g_assert(!hud_client_query_toolbar_item_active(query, HUD_CLIENT_QUERY_TOOLBAR_HELP));
  g_assert(!hud_client_query_toolbar_item_active(query, HUD_CLIENT_QUERY_TOOLBAR_PREFERENCES));
  g_assert(!hud_client_query_toolbar_item_active(query, HUD_CLIENT_QUERY_TOOLBAR_UNDO));

  /* Set an 'undo' item */
  const gchar * undo_toolbar[] = {"undo"};
  dbus_mock_update_property (connection, DBUS_NAME, QUERY_PATH, 
    "com.canonical.hud.query", "ToolbarItems", g_variant_new_strv(undo_toolbar, G_N_ELEMENTS(undo_toolbar)));
  hud_test_utils_process_mainloop (100); /* Let the property change propigate */

  g_assert(!hud_client_query_toolbar_item_active(query, HUD_CLIENT_QUERY_TOOLBAR_FULLSCREEN));
  g_assert(!hud_client_query_toolbar_item_active(query, HUD_CLIENT_QUERY_TOOLBAR_HELP));
  g_assert(!hud_client_query_toolbar_item_active(query, HUD_CLIENT_QUERY_TOOLBAR_PREFERENCES));
  g_assert(hud_client_query_toolbar_item_active(query, HUD_CLIENT_QUERY_TOOLBAR_UNDO));

  /* Set an 'invalid' item */
  const gchar * invalid_toolbar[] = {"invalid"};
  dbus_mock_update_property (connection, DBUS_NAME, QUERY_PATH, 
    "com.canonical.hud.query", "ToolbarItems", g_variant_new_strv(invalid_toolbar, G_N_ELEMENTS(invalid_toolbar)));
  hud_test_utils_process_mainloop (100); /* Let the property change propigate */

  g_assert(!hud_client_query_toolbar_item_active(query, HUD_CLIENT_QUERY_TOOLBAR_FULLSCREEN));
  g_assert(!hud_client_query_toolbar_item_active(query, HUD_CLIENT_QUERY_TOOLBAR_HELP));
  g_assert(!hud_client_query_toolbar_item_active(query, HUD_CLIENT_QUERY_TOOLBAR_PREFERENCES));
  g_assert(!hud_client_query_toolbar_item_active(query, HUD_CLIENT_QUERY_TOOLBAR_UNDO));

  /* Set all items */
  const gchar * full_toolbar[] = {"fullscreen", "undo", "help", "preferences"};
  dbus_mock_update_property (connection, DBUS_NAME, QUERY_PATH, 
    "com.canonical.hud.query", "ToolbarItems", g_variant_new_strv(full_toolbar, G_N_ELEMENTS(full_toolbar)));
  hud_test_utils_process_mainloop (100); /* Let the property change propigate */

  g_assert(hud_client_query_toolbar_item_active(query, HUD_CLIENT_QUERY_TOOLBAR_FULLSCREEN));
  g_assert(hud_client_query_toolbar_item_active(query, HUD_CLIENT_QUERY_TOOLBAR_HELP));
  g_assert(hud_client_query_toolbar_item_active(query, HUD_CLIENT_QUERY_TOOLBAR_PREFERENCES));
  g_assert(hud_client_query_toolbar_item_active(query, HUD_CLIENT_QUERY_TOOLBAR_UNDO));

  /* Check the array */
  GArray * toolbar = hud_client_query_get_active_toolbar(query);
  g_assert_cmpint(toolbar->len, ==, 4);

  gboolean found_undo = FALSE;
  gboolean found_help = FALSE;
  gboolean found_prefs = FALSE;
  gboolean found_full = FALSE;

  int i;
  for (i = 0; i < toolbar->len; i++) {
    switch (g_array_index(toolbar, int, i)) {
    case HUD_CLIENT_QUERY_TOOLBAR_FULLSCREEN:
      g_assert(!found_full);
      found_full = TRUE;
      break;
    case HUD_CLIENT_QUERY_TOOLBAR_PREFERENCES:
      g_assert(!found_prefs);
      found_prefs = TRUE;
      break;
    case HUD_CLIENT_QUERY_TOOLBAR_UNDO:
      g_assert(!found_undo);
      found_undo = TRUE;
      break;
    case HUD_CLIENT_QUERY_TOOLBAR_HELP:
      g_assert(!found_help);
      found_help = TRUE;
      break;
    default:
      g_assert_not_reached();
	}
  }


  g_assert(found_undo);
  g_assert(found_help);
  g_assert(found_prefs);
  g_assert(found_full);

  /* Clean up */
  g_object_unref(query);

  hud_test_utils_stop_hud_service (service, connection, results_model,
      appstack_model);
}

static void
test_suite (void)
{
  g_test_add_func ("/hud/client/query/create", test_query_create);
  g_test_add_func ("/hud/client/query/custom", test_query_custom);
  g_test_add_func ("/hud/client/query/update", test_query_update);
  g_test_add_func ("/hud/client/query/voice", test_query_voice);
  g_test_add_func ("/hud/client/query/update_app", test_query_update_app);
  g_test_add_func ("/hud/client/query/execute_command", test_query_execute_command);
  g_test_add_func ("/hud/client/query/execute_command_parameterized", test_query_execute_parameterized);
  g_test_add_func ("/hud/client/query/execute_command_toolbar", test_query_execute_toolbar);
  g_test_add_func ("/hud/client/query/toolbar_endabled", test_query_toolbar_enabled);
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
