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

#include <libhud-client/HudClient.h>
#include <common/DBusTypes.h>
#include <common/WindowStackInterface.h>

#include <QDebug>
#include <QDBusConnection>
#include <QString>
#include <QSignalSpy>
#include <QTestEventLoop>
#include <QAbstractListModel>
#include <libqtdbustest/QProcessDBusService.h>
#include <libqtdbustest/DBusTestRunner.h>
#include <libqtdbusmock/DBusMock.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace hud::common;
using namespace hud::client;
using namespace std;
using namespace testing;
using namespace QtDBusTest;
using namespace QtDBusMock;

namespace {

class TestHud: public Test {
protected:
	typedef QPair<QString, QString> ResultPair;

	TestHud() :
			mock(dbus) {

		mock.registerCustomMock(DBusTypes::BAMF_DBUS_NAME,
				DBusTypes::BAMF_MATCHER_DBUS_PATH, "org.ayatana.bamf.control",
				QDBusConnection::SessionBus);

		mock.registerCustomMock(DBusTypes::WINDOW_STACK_DBUS_NAME,
				DBusTypes::WINDOW_STACK_DBUS_PATH,
				ComCanonicalUnityWindowStackInterface::staticInterfaceName(),
				QDBusConnection::SessionBus);

		mock.registerCustomMock(DBusTypes::APPMENU_REGISTRAR_DBUS_NAME,
				DBusTypes::APPMENU_REGISTRAR_DBUS_PATH,
				"com.canonical.AppMenu.Registrar", QDBusConnection::SessionBus);

		dbus.startServices();
	}

	virtual ~TestHud() {
	}

	OrgFreedesktopDBusMockInterface & bamfMatcherMock() {
		return mock.mockInterface(DBusTypes::BAMF_DBUS_NAME,
				DBusTypes::BAMF_MATCHER_DBUS_PATH, "org.ayatana.bamf.control",
				QDBusConnection::SessionBus);
	}

	OrgFreedesktopDBusMockInterface & windowStackMock() {
		return mock.mockInterface(DBusTypes::WINDOW_STACK_DBUS_NAME,
				DBusTypes::WINDOW_STACK_DBUS_PATH,
				ComCanonicalUnityWindowStackInterface::staticInterfaceName(),
				QDBusConnection::SessionBus);
	}

	OrgFreedesktopDBusMockInterface & appmenuRegstrarMock() {
		return mock.mockInterface(DBusTypes::APPMENU_REGISTRAR_DBUS_NAME,
				DBusTypes::APPMENU_REGISTRAR_DBUS_PATH,
				"com.canonical.AppMenu.Registrar", QDBusConnection::SessionBus);
	}

	void startHud() {
		hud.reset(
				new QProcessDBusService(DBusTypes::HUD_SERVICE_DBUS_NAME,
						QDBusConnection::SessionBus,
						HUD_SERVICE_BINARY, QStringList()));
		hud->start(dbus.sessionConnection());
	}

	void startDBusMenu(const QString &name, const QString &path,
			const QString &json) {
		menuService.reset(
				new QProcessDBusService(name, QDBusConnection::SessionBus,
				DBUSMENU_JSON_LOADER, QStringList() << name << path << json));
		menuService->start(dbus.sessionConnection());
	}

	void startGMenu(const QString &name, const QString &path,
			const QString &model) {
		menuService.reset(
				new QProcessDBusService(name, QDBusConnection::SessionBus,
						model, QStringList() << name << path));
		menuService->start(dbus.sessionConnection());
	}

	QDBusConnection connection() {
		return dbus.sessionConnection();
	}

	static ResultPair result(const QAbstractListModel &results, uint i) {
		QModelIndex index = results.index(i);
		return ResultPair(results.data(index, 1).toString(),
				results.data(index, 3).toString());
	}

	DBusTestRunner dbus;

	DBusMock mock;

	QSharedPointer<QProcessDBusService> hud;

	QSharedPointer<QProcessDBusService> menuService;
};

TEST_F(TestHud, SearchDBusMenuContext) {
	startDBusMenu("menu.name", "/menu", JSON_SOURCE);

	windowStackMock().AddMethod(DBusTypes::WINDOW_STACK_DBUS_NAME,
			"GetWindowStack", "", "a(usbu)", "ret = [(0, 'app0', True, 0)]").waitForFinished();

	// There are no GMenus in this test
	windowStackMock().AddMethod(DBusTypes::WINDOW_STACK_DBUS_NAME,
			"GetWindowProperties", "usas", "as", "ret = []\n"
					"for arg in args[2]:\n"
					"  ret.append('')").waitForFinished();

	appmenuRegstrarMock().AddMethod(DBusTypes::APPMENU_REGISTRAR_DBUS_NAME,
			"GetMenuForWindow", "u", "so", "ret = ('menu.name', '/menu')").waitForFinished();

	startHud();

	HudClient client;
	QSignalSpy modelsChangedSpy(&client, SIGNAL(modelsChanged()));
	modelsChangedSpy.wait();

	QSignalSpy countChangedSpy(client.results(), SIGNAL(countChanged()));
	client.setQuery("piece hook");
	countChangedSpy.wait();

	QAbstractListModel &results(*client.results());
	ASSERT_EQ(2, results.rowCount());
	EXPECT_EQ(ResultPair("swift sad", "piece hook"), result(results, 0));
	EXPECT_EQ(ResultPair("stray slash", "piece hook"), result(results, 1));
}

TEST_F(TestHud, SearchDBusMenuOneResult) {
	startDBusMenu("menu.name", "/menu", JSON_SHORTCUTS);

	windowStackMock().AddMethod(DBusTypes::WINDOW_STACK_DBUS_NAME,
			"GetWindowStack", "", "a(usbu)", "ret = [(0, 'app0', True, 0)]").waitForFinished();

	// There are no GMenus in this test
	windowStackMock().AddMethod(DBusTypes::WINDOW_STACK_DBUS_NAME,
			"GetWindowProperties", "usas", "as", "ret = []\n"
					"for arg in args[2]:\n"
					"  ret.append('')").waitForFinished();

	appmenuRegstrarMock().AddMethod(DBusTypes::APPMENU_REGISTRAR_DBUS_NAME,
			"GetMenuForWindow", "u", "so", "ret = ('menu.name', '/menu')").waitForFinished();

	startHud();

	HudClient client;
	QSignalSpy modelsChangedSpy(&client, SIGNAL(modelsChanged()));
	modelsChangedSpy.wait();

	QSignalSpy countChangedSpy(client.results(), SIGNAL(countChanged()));
	client.setQuery("quit");
	countChangedSpy.wait();

	QAbstractListModel &results(*client.results());
	ASSERT_EQ(1, results.rowCount());
	EXPECT_EQ(ResultPair("Quiter", ""), result(results, 0));

	QSignalSpy executedSpy(&client, SIGNAL(commandExecuted()));
	client.executeCommand(0);
	executedSpy.wait();
}

TEST_F(TestHud, SearchGMenuOneResult) {
	startGMenu("menu.name", "/menu", MODEL_SHORTCUTS);

	windowStackMock().AddMethod(DBusTypes::WINDOW_STACK_DBUS_NAME,
			"GetWindowStack", "", "a(usbu)", "ret = [(0, 'app0', True, 0)]").waitForFinished();

	windowStackMock().AddMethod(DBusTypes::WINDOW_STACK_DBUS_NAME,
			"GetWindowProperties", "usas", "as", "ret = []\n"
					"if args[0] == 0:\n"
					"  ret.append('menu.name')\n"
					"  ret.append('')\n"
					"  ret.append('/menu')\n"
					"  ret.append('')\n"
					"  ret.append('')\n"
					"  ret.append('')\n"
					"else:\n"
					"  for arg in args[2]:\n"
					"    ret.append('')").waitForFinished();

	// There are no DBusMenus in this test
	appmenuRegstrarMock().AddMethod(DBusTypes::APPMENU_REGISTRAR_DBUS_NAME,
			"GetMenuForWindow", "u", "so", "ret = ('', '/')").waitForFinished();

	startHud();

	HudClient client;
	QSignalSpy modelsChangedSpy(&client, SIGNAL(modelsChanged()));
	modelsChangedSpy.wait();

	QSignalSpy countChangedSpy(client.results(), SIGNAL(countChanged()));
	client.setQuery("closr");
	countChangedSpy.wait();
	countChangedSpy.wait();

	QAbstractListModel &results(*client.results());
	ASSERT_EQ(1, results.rowCount());
	EXPECT_EQ(ResultPair("Close", ""), result(results, 0));
}

} // namespace
