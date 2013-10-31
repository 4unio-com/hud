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

#include <common/ApplicationInterface.h>
#include <service/Factory.h>
#include <service/ApplicationImpl.h>
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

class TestApplication: public Test {
protected:
	TestApplication() :
			mock(dbus) {
		factory.setSessionBus(dbus.sessionConnection());

	}

	virtual ~TestApplication() {
	}

	DBusTestRunner dbus;

	DBusMock mock;

	NiceMock<MockFactory> factory;
};

TEST_F(TestApplication, DBusInterfaceIsExported) {
	ApplicationImpl application(1234, "application-id", factory,
			dbus.sessionConnection());

	ComCanonicalHudApplicationInterface applicationInterface(
			dbus.sessionConnection().baseService(),
			DBusTypes::applicationPath(1234), dbus.sessionConnection());

	ASSERT_TRUE(applicationInterface.isValid());

	//FIXME desktop path will return something when it's actually implemented
	EXPECT_EQ(QString(), applicationInterface.desktopPath());
}

TEST_F(TestApplication, AddsWindow) {
	ApplicationImpl application(1234, "application-id", factory,
			dbus.sessionConnection());
	EXPECT_TRUE(application.isEmpty());

	QSharedPointer<MockWindow> window(new NiceMock<MockWindow>());

	EXPECT_CALL(factory, newWindow(4567, QString("application-id"))).WillOnce(
			Return(window));
	application.addWindow(4567);
	EXPECT_FALSE(application.isEmpty());
}

TEST_F(TestApplication, HandlesDeleteUnknownWindow) {
	ApplicationImpl application(1234, "application-id", factory,
			dbus.sessionConnection());
	EXPECT_TRUE(application.isEmpty());

	QSharedPointer<MockWindow> window(new NiceMock<MockWindow>());
	application.removeWindow(4567);
	EXPECT_TRUE(application.isEmpty());
}

TEST_F(TestApplication, DeletesWindow) {
	ApplicationImpl application(1234, "application-id", factory,
			dbus.sessionConnection());

	QSharedPointer<MockWindow> window0(new NiceMock<MockWindow>());
	QSharedPointer<MockWindow> window1(new NiceMock<MockWindow>());

	EXPECT_CALL(factory, newWindow(0, QString("application-id"))).WillOnce(
			Return(window0));
	application.addWindow(0);
	EXPECT_FALSE(application.isEmpty());

	EXPECT_CALL(factory, newWindow(1, QString("application-id"))).WillOnce(
			Return(window1));
	application.addWindow(1);
	EXPECT_FALSE(application.isEmpty());

	application.removeWindow(0);
	EXPECT_FALSE(application.isEmpty());

	application.removeWindow(1);
	EXPECT_TRUE(application.isEmpty());
}

} // namespace
