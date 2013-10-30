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

#include <service/Factory.h>
#include <service/ApplicationListImpl.h>
#include <unit/service/Mocks.h>

#include <QDebug>
#include <libqtdbustest/DBusTestRunner.h>
#include <libqtdbusmock/DBusMock.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace std;
using namespace testing;
using namespace QtDBusTest;
using namespace QtDBusMock;
using namespace hud::common;
using namespace hud::service;
using namespace hud::service::test;

namespace {

class TestApplicationList: public Test {
protected:
	TestApplicationList() :
			mock(dbus) {

		factory.setSessionBus(dbus.sessionConnection());

		mock.registerCustomMock(DBusTypes::WINDOW_STACK_DBUS_NAME,
				DBusTypes::WINDOW_STACK_DBUS_PATH,
				ComCanonicalUnityWindowStackInterface::staticInterfaceName(),
				QDBusConnection::SessionBus);

		dbus.startServices();

		windowStack.reset(
				new ComCanonicalUnityWindowStackInterface(
						DBusTypes::WINDOW_STACK_DBUS_NAME,
						DBusTypes::WINDOW_STACK_DBUS_PATH,
						dbus.sessionConnection()));

	}

	virtual ~TestApplicationList() {
	}

	virtual OrgFreedesktopDBusMockInterface & windowStackMock() {
		return mock.mockInterface(DBusTypes::WINDOW_STACK_DBUS_NAME,
				DBusTypes::WINDOW_STACK_DBUS_PATH,
				ComCanonicalUnityWindowStackInterface::staticInterfaceName(),
				QDBusConnection::SessionBus);
	}

	DBusTestRunner dbus;

	DBusMock mock;

	NiceMock<MockFactory> factory;

	QSharedPointer<ComCanonicalUnityWindowStackInterface> windowStack;
};

TEST_F(TestApplicationList, CreatesWindowsOnStartup) {
	windowStackMock().AddMethod(DBusTypes::WINDOW_STACK_DBUS_NAME,
			"GetWindowStack", "", "a(usbu)", "ret = [(0, 'app0', True, 0)]").waitForFinished();

	QSharedPointer<MockApplication> application(
			new NiceMock<MockApplication>());
	QDBusObjectPath path(QDBusObjectPath("/path/app/0"));
	ON_CALL(*application, path()).WillByDefault(ReturnRef(path));

	EXPECT_CALL(factory, newApplication(0, QString("app0"))).WillOnce(
			Return(application));

	ApplicationListImpl applicationList(factory, windowStack);

	ASSERT_EQ(1, applicationList.applications().size());
	EXPECT_EQ(NameObject("app0", path), applicationList.applications().at(0));
}

} // namespace
