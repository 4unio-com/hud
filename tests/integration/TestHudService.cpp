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

#include <common/DBusTypes.h>

#include <QDebug>
#include <libqtdbustest/QProcessDBusService.h>
#include <libqtdbustest/DBusTestRunner.h>
#include <libqtdbusmock/DBusMock.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace hud::common;
using namespace std;
using namespace testing;
using namespace QtDBusTest;
using namespace QtDBusMock;

namespace {

class TestHud: public Test {
protected:
	TestHud() :
			mock(dbus) {

		mock.registerCustomMock(DBusTypes::BAMF_DBUS_NAME,
				DBusTypes::BAMF_MATCHER_DBUS_PATH, "org.ayatana.bamf.control",
				QDBusConnection::SessionBus);

		// It seems that QApplication needs this
		mock.registerCustomMock("org.gtk.vfs.Daemon", "/org/gtk/vfs/Daemon",
				"org.gtk.vfs.Daemon", QDBusConnection::SessionBus);

		dbus.startServices();
	}

	virtual ~TestHud() {
	}

	OrgFreedesktopDBusMockInterface & bamfMatcherMock() {
		return mock.mockInterface(DBusTypes::BAMF_DBUS_NAME,
				DBusTypes::BAMF_MATCHER_DBUS_PATH, "org.ayatana.bamf.control",
				QDBusConnection::SessionBus);
	}

	void startHud() {
		hud.reset(
				new QProcessDBusService(DBusTypes::HUD_SERVICE_DBUS_NAME,
						QDBusConnection::SessionBus,
						HUD_SERVICE_BINARY, QStringList()));
		hud->start(dbus.sessionConnection());
	}

	DBusTestRunner dbus;

	DBusMock mock;

	QSharedPointer<QProcessDBusService> hud;
};

TEST_F(TestHud, OpenCloseQuery) {
	startHud();
}

} // namespace
