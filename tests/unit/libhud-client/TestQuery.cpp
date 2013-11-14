/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Pete Woods <pete.woods@canonical.com>
 */

#include <testutils/MockHudService.h>
#include <testutils/RawDBusTransformer.h>
#include <libhud-client/hud-client.h>
#include <common/shared-values.h>

#include <libqtdbustest/DBusTestRunner.h>
#include <libqtdbusmock/DBusMock.h>
#include <QSignalSpy>
#include <QTestEventLoop>
#include <gtest/gtest.h>

using namespace testing;
using namespace QtDBusTest;
using namespace QtDBusMock;
using namespace hud::common;
using namespace hud::testutils;

namespace {

class TestQuery: public QObject, public Test {
Q_OBJECT

protected:
	TestQuery() :
			mock(dbus), hud(dbus, mock) {
		dbus.startServices();
		hud.loadMethods();

		connection = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, NULL);
	}

	virtual ~TestQuery() {
		g_object_unref(connection);
	}

	void EXPECT_CALL(const QList<QVariantList> &spy, int index,
			const QString &name, const QVariantList &args) {
		QVariant args2(QVariant::fromValue(args));
		ASSERT_LT(index, spy.size());
		const QVariantList &call(spy.at(index));
		EXPECT_EQ(name, call.at(0).toString());
		EXPECT_EQ(args2.toString().toStdString(),
				call.at(1).toString().toStdString());
	}

	static void callbackModelsReady(HudClientQuery *query, gpointer user_data) {
		Q_UNUSED(query);
		TestQuery *self = static_cast<TestQuery*>(user_data);
		self->modelsReady();
	}

	static void callbackVoiceQueryFinished(HudClientQuery *query,
			GDBusMethodInvocation *invocation, gpointer user_data) {
		Q_UNUSED(query);
		Q_UNUSED(invocation);
		TestQuery *self = static_cast<TestQuery*>(user_data);
		self->queryFinished();
	}

Q_SIGNALS:
	void modelsReady();

	void queryFinished();

protected:

	DBusTestRunner dbus;

	DBusMock mock;

	MockHudService hud;

	GDBusConnection *connection;
};

TEST_F(TestQuery, Create) {
	QSignalSpy querySpy(this, SIGNAL(modelsReady()));

	/* Create a query */
	HudClientQuery * query = hud_client_query_new("test");

	/* Wait for the models to be ready */
	g_signal_connect(G_OBJECT(query), HUD_CLIENT_QUERY_SIGNAL_MODELS_CHANGED,
			G_CALLBACK(callbackModelsReady), this);
	querySpy.wait();

	/* Check the models */
	ASSERT_TRUE(DEE_IS_MODEL(hud_client_query_get_results_model(query)));
	ASSERT_TRUE(DEE_IS_MODEL(hud_client_query_get_appstack_model(query)));

	EXPECT_STREQ("test", hud_client_query_get_query(query));

	HudClientConnection * client_connection = NULL;
	gchar * search = NULL;

	g_object_get(G_OBJECT(query), "query", &search, "connection",
			&client_connection, NULL);

	EXPECT_STREQ("test", search);
	ASSERT_TRUE(HUD_CLIENT_IS_CONNECTION(client_connection));

	g_free(search);

	g_object_unref(query);

	g_object_unref(client_connection);
}

TEST_F(TestQuery, Custom) {
	QSignalSpy querySpy(this, SIGNAL(modelsReady()));

	/* Create a connection */
	HudClientConnection * client_connection = hud_client_connection_new(
	DBUS_NAME, DBUS_PATH);
	ASSERT_TRUE(HUD_CLIENT_IS_CONNECTION(client_connection));

	/* Create a query */
	HudClientQuery * query = hud_client_query_new_for_connection("test",
			client_connection);
	ASSERT_TRUE(HUD_CLIENT_IS_QUERY(query));

	/* Wait for the models to be ready */
	g_signal_connect(G_OBJECT(query), HUD_CLIENT_QUERY_SIGNAL_MODELS_CHANGED,
			G_CALLBACK(callbackModelsReady), this);
	querySpy.wait();

	/* Make sure it has models */
	ASSERT_TRUE(DEE_IS_MODEL(hud_client_query_get_results_model(query)));
	ASSERT_TRUE(DEE_IS_MODEL(hud_client_query_get_appstack_model(query)));

	EXPECT_STREQ("test", hud_client_query_get_query(query));

	/* Make sure the connection is the same */
	HudClientConnection *testcon = NULL;

	g_object_get(G_OBJECT(query), "connection", &testcon, NULL);

	ASSERT_TRUE(HUD_CLIENT_IS_CONNECTION(testcon));
	ASSERT_EQ(testcon, client_connection);
	g_object_unref(testcon);

	/* Clean up */
	g_object_unref(query);
	g_object_unref(client_connection);
}

TEST_F(TestQuery, Update) {
	QSignalSpy remoteQuerySpy(&hud.queryInterface(),
	SIGNAL(MethodCalled(const QString &, const QVariantList &)));

	QSignalSpy querySpy(this, SIGNAL(modelsReady()));

	/* Create a query */
	HudClientQuery *query = hud_client_query_new("test");
	g_assert(HUD_CLIENT_IS_QUERY(query));

	/* Wait for the models to be ready */
	g_signal_connect(G_OBJECT(query), HUD_CLIENT_QUERY_SIGNAL_MODELS_CHANGED,
			G_CALLBACK(callbackModelsReady), this);
	querySpy.wait();

	hud_client_query_set_query(query, "test2");
	EXPECT_STREQ("test2", hud_client_query_get_query(query));

	remoteQuerySpy.wait();
	EXPECT_CALL(remoteQuerySpy, 0, "UpdateQuery", QVariantList() << "test2");

	g_object_unref(query);
}

TEST_F(TestQuery, Voice) {
	QSignalSpy remoteQuerySpy(&hud.queryInterface(),
	SIGNAL(MethodCalled(const QString &, const QVariantList &)));

	QSignalSpy querySpy(this, SIGNAL(modelsReady()));

	/* Create a query */
	HudClientQuery *query = hud_client_query_new("test");
	ASSERT_TRUE(HUD_CLIENT_IS_QUERY(query));

	/* Wait for the models to be ready */
	g_signal_connect(G_OBJECT(query), HUD_CLIENT_QUERY_SIGNAL_MODELS_CHANGED,
			G_CALLBACK(callbackModelsReady), this);
	querySpy.wait();

	/* Call the voice query */
	g_signal_connect(G_OBJECT(query), "voice-query-finished",
			G_CALLBACK(callbackVoiceQueryFinished), this);

	QSignalSpy queryFinishedSpy(this, SIGNAL(queryFinished()));

	hud_client_query_voice_query(query);

	remoteQuerySpy.wait();
	EXPECT_CALL(remoteQuerySpy, 0, "VoiceQuery", QVariantList());

	queryFinishedSpy.wait();
	EXPECT_EQ(1, queryFinishedSpy.size());

	g_object_unref(query);
}

TEST_F(TestQuery, UpdateApp) {
	QSignalSpy remoteQuerySpy(&hud.queryInterface(),
	SIGNAL(MethodCalled(const QString &, const QVariantList &)));

	QSignalSpy querySpy(this, SIGNAL(modelsReady()));

	/* Create a query */
	HudClientQuery *query = hud_client_query_new("test");
	g_assert(HUD_CLIENT_IS_QUERY(query));

	/* Wait for the models to be ready */
	g_signal_connect(G_OBJECT(query), HUD_CLIENT_QUERY_SIGNAL_MODELS_CHANGED,
			G_CALLBACK(callbackModelsReady), this);
	querySpy.wait();

	/* Set App ID */
	hud_client_query_set_appstack_app(query, "application-id");

	remoteQuerySpy.wait();
	EXPECT_CALL(remoteQuerySpy, 0, "UpdateApp",
			QVariantList() << "application-id");

	g_object_unref(query);
}

TEST_F(TestQuery, ExecuteCommand) {
	QSignalSpy remoteQuerySpy(&hud.queryInterface(),
	SIGNAL(MethodCalled(const QString &, const QVariantList &)));

	QSignalSpy querySpy(this, SIGNAL(modelsReady()));

	/* Create a query */
	HudClientQuery *query = hud_client_query_new("test");
	g_assert(HUD_CLIENT_IS_QUERY(query));

	/* Wait for the models to be ready */
	g_signal_connect(G_OBJECT(query), HUD_CLIENT_QUERY_SIGNAL_MODELS_CHANGED,
			G_CALLBACK(callbackModelsReady), this);
	querySpy.wait();

	/* Execute a command */
	hud_client_query_execute_command(query,
			g_variant_new_variant(g_variant_new_uint64(4321)), 1234);

	remoteQuerySpy.wait();
	EXPECT_CALL(remoteQuerySpy, 0, "ExecuteCommand",
			QVariantList() << qulonglong(4321) << uint(1234));

	g_object_unref(query);
}

TEST_F(TestQuery, ExecuteParameterized) {
	QSignalSpy remoteQuerySpy(&hud.queryInterface(),
	SIGNAL(MethodCalled(const QString &, const QVariantList &)));

	QSignalSpy querySpy(this, SIGNAL(modelsReady()));

	/* Create a query */
	HudClientQuery *query = hud_client_query_new("test");
	g_assert(HUD_CLIENT_IS_QUERY(query));

	/* Wait for the models to be ready */
	g_signal_connect(G_OBJECT(query), HUD_CLIENT_QUERY_SIGNAL_MODELS_CHANGED,
			G_CALLBACK(callbackModelsReady), this);
	querySpy.wait();

	/* Execute a parameterized command */
	HudClientParam *param = hud_client_query_execute_param_command(query,
			g_variant_new_variant(g_variant_new_uint64(4321)), 1234);

	remoteQuerySpy.wait();
	EXPECT_CALL(remoteQuerySpy, 0, "ExecuteParameterized",
			QVariantList() << qulonglong(4321) << uint(1234));

	g_object_unref(param);
	g_object_unref(query);
}

TEST_F(TestQuery, ExecuteToolbar) {
//  g_test_log_set_fatal_handler(no_dee_add_match, NULL);
//
//  DbusTestService *service = NULL;
//  GDBusConnection *connection = NULL;
//  DeeModel *results_model = NULL;
//  DeeModel *appstack_model = NULL;
//
//  hud_test_utils_start_hud_service (&service, &connection, &results_model,
//      &appstack_model);
//
//  /* Create a query */
//  HudClientQuery *query = hud_client_query_new("test");
//  g_assert(HUD_CLIENT_IS_QUERY(query));
//
//  /* Wait for the models to be ready */
//  GMainLoop * loop = g_main_loop_new(NULL, FALSE);
//  gulong sig = g_timeout_add_seconds(TEST_DEFAULT_TIMEOUT_S, fail_quit, loop);
//
//  g_signal_connect(G_OBJECT(query), HUD_CLIENT_QUERY_SIGNAL_MODELS_CHANGED, G_CALLBACK(test_query_create_models_ready), loop);
//
//  g_source_remove(sig);
//
//  g_main_loop_run(loop);
//  g_main_loop_unref(loop);
//
//  /* Start attacking the toolbar */
//  hud_client_query_execute_toolbar_item (query,
//      HUD_CLIENT_QUERY_TOOLBAR_FULLSCREEN, 12345);
//  dbus_mock_assert_method_call_results (connection, DBUS_NAME, QUERY_PATH,
//      "ExecuteToolbar",
//      "\\(\\[\\(\\d+, \\[<'fullscreen'>, <uint32 12345>\\]\\)\\],\\)");
//  dbus_mock_clear_method_calls (connection, DBUS_NAME, QUERY_PATH);
//
//  hud_client_query_execute_toolbar_item (query,
//      HUD_CLIENT_QUERY_TOOLBAR_HELP, 12);
//  dbus_mock_assert_method_call_results (connection, DBUS_NAME, QUERY_PATH,
//      "ExecuteToolbar",
//      "\\(\\[\\(\\d+, \\[<'help'>, <uint32 12>\\]\\)\\],\\)");
//  dbus_mock_clear_method_calls (connection, DBUS_NAME, QUERY_PATH);
//
//  hud_client_query_execute_toolbar_item (query,
//      HUD_CLIENT_QUERY_TOOLBAR_PREFERENCES, 312);
//  dbus_mock_assert_method_call_results (connection, DBUS_NAME, QUERY_PATH,
//      "ExecuteToolbar",
//      "\\(\\[\\(\\d+, \\[<'preferences'>, <uint32 312>\\]\\)\\],\\)");
//  dbus_mock_clear_method_calls (connection, DBUS_NAME, QUERY_PATH);
//
//  hud_client_query_execute_toolbar_item (query,
//      HUD_CLIENT_QUERY_TOOLBAR_UNDO, 53312);
//  dbus_mock_assert_method_call_results (connection, DBUS_NAME, QUERY_PATH,
//      "ExecuteToolbar",
//      "\\(\\[\\(\\d+, \\[<'undo'>, <uint32 53312>\\]\\)\\],\\)");
//  dbus_mock_clear_method_calls (connection, DBUS_NAME, QUERY_PATH);
//
//  g_object_unref(query);
//
//  hud_test_utils_stop_hud_service (service, connection, results_model,
//      appstack_model);
}

TEST_F(TestQuery, ToolbarEnabled) {
//  g_test_log_set_fatal_handler(no_dee_add_match, NULL);
//
//  DbusTestService *service = NULL;
//  GDBusConnection *connection = NULL;
//  DeeModel *results_model = NULL;
//  DeeModel *appstack_model = NULL;
//
//  hud_test_utils_start_hud_service (&service, &connection, &results_model,
//      &appstack_model);
//
//  /* Create a query */
//  HudClientQuery *query = hud_client_query_new("test");
//  g_assert(HUD_CLIENT_IS_QUERY(query));
//
//  /* Wait for the models to be ready */
//  GMainLoop * loop = g_main_loop_new(NULL, FALSE);
//  gulong sig = g_timeout_add_seconds(TEST_DEFAULT_TIMEOUT_S, fail_quit, loop);
//
//  g_signal_connect(G_OBJECT(query), HUD_CLIENT_QUERY_SIGNAL_MODELS_CHANGED, G_CALLBACK(test_query_create_models_ready), loop);
//
//  g_source_remove(sig);
//
//  g_main_loop_run(loop);
//  g_main_loop_unref(loop);
//
//  /* Test toolbar disabled */
//  g_assert(!hud_client_query_toolbar_item_active(query, HUD_CLIENT_QUERY_TOOLBAR_FULLSCREEN));
//  g_assert(!hud_client_query_toolbar_item_active(query, HUD_CLIENT_QUERY_TOOLBAR_HELP));
//  g_assert(!hud_client_query_toolbar_item_active(query, HUD_CLIENT_QUERY_TOOLBAR_PREFERENCES));
//  g_assert(!hud_client_query_toolbar_item_active(query, HUD_CLIENT_QUERY_TOOLBAR_UNDO));
//
//  /* Set an 'undo' item */
//  const gchar * undo_toolbar[] = {"undo"};
//  dbus_mock_update_property (connection, DBUS_NAME, QUERY_PATH,
//    "com.canonical.hud.query", "ToolbarItems", g_variant_new_strv(undo_toolbar, G_N_ELEMENTS(undo_toolbar)));
//  hud_test_utils_process_mainloop (100); /* Let the property change propigate */
//
//  g_assert(!hud_client_query_toolbar_item_active(query, HUD_CLIENT_QUERY_TOOLBAR_FULLSCREEN));
//  g_assert(!hud_client_query_toolbar_item_active(query, HUD_CLIENT_QUERY_TOOLBAR_HELP));
//  g_assert(!hud_client_query_toolbar_item_active(query, HUD_CLIENT_QUERY_TOOLBAR_PREFERENCES));
//  g_assert(hud_client_query_toolbar_item_active(query, HUD_CLIENT_QUERY_TOOLBAR_UNDO));
//
//  /* Set an 'invalid' item */
//  const gchar * invalid_toolbar[] = {"invalid"};
//  dbus_mock_update_property (connection, DBUS_NAME, QUERY_PATH,
//    "com.canonical.hud.query", "ToolbarItems", g_variant_new_strv(invalid_toolbar, G_N_ELEMENTS(invalid_toolbar)));
//  hud_test_utils_process_mainloop (100); /* Let the property change propigate */
//
//  g_assert(!hud_client_query_toolbar_item_active(query, HUD_CLIENT_QUERY_TOOLBAR_FULLSCREEN));
//  g_assert(!hud_client_query_toolbar_item_active(query, HUD_CLIENT_QUERY_TOOLBAR_HELP));
//  g_assert(!hud_client_query_toolbar_item_active(query, HUD_CLIENT_QUERY_TOOLBAR_PREFERENCES));
//  g_assert(!hud_client_query_toolbar_item_active(query, HUD_CLIENT_QUERY_TOOLBAR_UNDO));
//
//  /* Set all items */
//  const gchar * full_toolbar[] = {"fullscreen", "undo", "help", "preferences"};
//  dbus_mock_update_property (connection, DBUS_NAME, QUERY_PATH,
//    "com.canonical.hud.query", "ToolbarItems", g_variant_new_strv(full_toolbar, G_N_ELEMENTS(full_toolbar)));
//  hud_test_utils_process_mainloop (100); /* Let the property change propigate */
//
//  g_assert(hud_client_query_toolbar_item_active(query, HUD_CLIENT_QUERY_TOOLBAR_FULLSCREEN));
//  g_assert(hud_client_query_toolbar_item_active(query, HUD_CLIENT_QUERY_TOOLBAR_HELP));
//  g_assert(hud_client_query_toolbar_item_active(query, HUD_CLIENT_QUERY_TOOLBAR_PREFERENCES));
//  g_assert(hud_client_query_toolbar_item_active(query, HUD_CLIENT_QUERY_TOOLBAR_UNDO));
//
//  /* Check the array */
//  GArray * toolbar = hud_client_query_get_active_toolbar(query);
//  g_assert_cmpint(toolbar->len, ==, 4);
//
//  gboolean found_undo = FALSE;
//  gboolean found_help = FALSE;
//  gboolean found_prefs = FALSE;
//  gboolean found_full = FALSE;
//
//  int i;
//  for (i = 0; i < toolbar->len; i++) {
//    switch (g_array_index(toolbar, int, i)) {
//    case HUD_CLIENT_QUERY_TOOLBAR_FULLSCREEN:
//      g_assert(!found_full);
//      found_full = TRUE;
//      break;
//    case HUD_CLIENT_QUERY_TOOLBAR_PREFERENCES:
//      g_assert(!found_prefs);
//      found_prefs = TRUE;
//      break;
//    case HUD_CLIENT_QUERY_TOOLBAR_UNDO:
//      g_assert(!found_undo);
//      found_undo = TRUE;
//      break;
//    case HUD_CLIENT_QUERY_TOOLBAR_HELP:
//      g_assert(!found_help);
//      found_help = TRUE;
//      break;
//    default:
//      g_assert_not_reached();
//	}
//  }
//
//
//  g_assert(found_undo);
//  g_assert(found_help);
//  g_assert(found_prefs);
//  g_assert(found_full);
//
//  /* Clean up */
//  g_object_unref(query);
//
//  hud_test_utils_stop_hud_service (service, connection, results_model,
//      appstack_model);
}

}

#include "TestQuery.moc"
