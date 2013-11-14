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
using namespace QtDBusTest;
using namespace QtDBusMock;

MockHudService::MockHudService(DBusTestRunner &dbus, DBusMock &mock) :
		m_dbus(dbus), m_mock(mock) {
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

OrgFreedesktopDBusMockInterface & MockHudService::hudInterface() {
	return m_mock.mockInterface(DBusTypes::HUD_SERVICE_DBUS_NAME,
			DBusTypes::HUD_SERVICE_DBUS_PATH, DBusTypes::HUD_SERVICE_DBUS_NAME,
			QDBusConnection::SessionBus);
}

OrgFreedesktopDBusMockInterface & MockHudService::applicationInterface() {
	//TODO Changed to be the same as the method above when lp:XXXX is fixed
	if (m_applicationInterface.isNull()) {
		m_applicationInterface.reset(
				new OrgFreedesktopDBusMockInterface(
						DBusTypes::HUD_SERVICE_DBUS_NAME, "/app/object",
						m_dbus.sessionConnection()));
	}
	return *m_applicationInterface;
}

void MockHudService::loadMethods() {
	OrgFreedesktopDBusMockInterface &hud(hudInterface());

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

	// Mock application
	{
		QVariantMap properties;
		QList<Method> methods;
		addMethod(methods, "AddSources", "a(usso)a(uso)", "", "");
		hud.AddObject("/app/object", "com.canonical.hud.Application",
				properties, methods).waitForFinished();
	}

	/* query */
	hud.AddMethod(DBusTypes::HUD_SERVICE_DBUS_NAME, "CreateQuery", "s", "ossi",
			"ret = ('/com/canonical/hud/query0', 'com.canonical.hud.query0.results', 'com.canonical.hud.query0.appstack', dbus.Int32(0))").waitForFinished();

	/* id */
	hud.AddMethod(DBusTypes::HUD_SERVICE_DBUS_NAME, "RegisterApplication", "s",
			"o", "ret = ('/app/object')").waitForFinished();
}

MockHudService::~MockHudService() {
}

