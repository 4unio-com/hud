/*
 * MockHudService.cpp
 *
 *  Created on: 14 Nov 2013
 *      Author: pete
 */

#include <testutils/MockHudService.h>
#include <common/AppstackModel.h>
#include <common/DBusTypes.h>
#include <common/ResultsModel.h>

using namespace hud::common;
using namespace hud::testutils;
using namespace QtDBusMock;

MockHudService::MockHudService(DBusMock &mock) :
		m_mock(mock) {
	mock.registerCustomMock(DBusTypes::HUD_SERVICE_DBUS_NAME,
			DBusTypes::HUD_SERVICE_DBUS_PATH, DBusTypes::HUD_SERVICE_DBUS_NAME,
			QDBusConnection::SessionBus);
}

static void addMethod(QList<Method> &methods, const QString &name,
		const QString &inSig, const QString &outSig, const QString &code) {
	Method method;
	method.setName(name);
	method.setInSig(inSig);
	method.setOutSig(outSig);
	method.setCode(code);
	methods << method;
}

void MockHudService::loadMethods() {
	OrgFreedesktopDBusMockInterface &hud = m_mock.mockInterface(
			DBusTypes::HUD_SERVICE_DBUS_NAME, DBusTypes::HUD_SERVICE_DBUS_PATH,
			DBusTypes::HUD_SERVICE_DBUS_NAME, QDBusConnection::SessionBus);

	QString queryPath("/com/canonical/hud/query0");

	{
		QVariantMap properties;
		properties["ResultsModel"] = "com.canonical.hud.query0.results";
		properties["AppstackModel"] = "com.canonical.hud.query0.appstack";
		properties["ToolbarItems"] = "";

		QList<Method> methods;
		addMethod(methods, "UpdateQuery", "s", "i", "ret = 1");
		addMethod(methods, "VoiceQuery", "", "is", "ret = (1, 'voice query')");
		addMethod(methods, "UpdateApp", "s", "i", "ret = 1");
		addMethod(methods, "CloseQuery", "", "", "");
		addMethod(methods, "ExecuteCommand", "vu", "", "");
		addMethod(methods, "ExecuteParameterized", "vu", "sooi",
				"ret = ('action', '/action/path', '/model/path', 1)");
		addMethod(methods, "ExecuteToolbar", "su", "", "");

		hud.AddObject(queryPath, "com.canonical.hud.query", properties, methods).waitForFinished();
	}

	m_results.reset(new ResultsModel(0));
	m_appstack.reset(new AppstackModel(0));

	/* query */
	hud.AddMethod(DBusTypes::HUD_SERVICE_DBUS_NAME, "CreateQuery", "s", "ossi",
			"ret = ('/com/canonical/hud/query0', 'com.canonical.hud.query0.results', 'com.canonical.hud.query0.appstack', dbus.Int32(0))").waitForFinished();

	/* id */
	hud.AddMethod(DBusTypes::HUD_SERVICE_DBUS_NAME, "RegisterApplication", "s",
			"o", "ret = ('/app/object')").waitForFinished();
}

MockHudService::~MockHudService() {
}

//const gchar *QUERY_PATH = "/com/canonical/hud/query0";
//
//const gchar * results_model_schema[] = {
//  "v", /* Command ID */
//  "s", /* Command Name */
//  "a(ii)", /* Highlights in command name */
//  "s", /* Description */
//  "a(ii)", /* Highlights in description */
//  "s", /* Shortcut */
//  "u", /* Distance */
//  "b", /* Parameterized */
//};
//
//const gchar * appstack_model_schema[] = {
//  "s", /* Application ID */
//  "s", /* Icon Name */
//  "i", /* Item Type */
//};
//
//void
//hud_test_utils_add_result (DeeModel *results_model, guint64 id,
//    const gchar *command, const gchar *description, const gchar *shortcut,
//    guint32 distance, gboolean parameterized)
//{
//  GVariant * columns[G_N_ELEMENTS(results_model_schema) + 1];
//  columns[0] = g_variant_new_variant(g_variant_new_uint64(id));
//  columns[1] = g_variant_new_string(command);
//  columns[2] = g_variant_new_array(G_VARIANT_TYPE("(ii)"), NULL, 0);
//  columns[3] = g_variant_new_string(description);
//  columns[4] = g_variant_new_array(G_VARIANT_TYPE("(ii)"), NULL, 0);
//  columns[5] = g_variant_new_string(shortcut);
//  columns[6] = g_variant_new_uint32(distance);
//  columns[7] = g_variant_new_boolean(parameterized);
//  columns[8] = NULL;
//
//  dee_model_append_row (results_model, columns);
//}

