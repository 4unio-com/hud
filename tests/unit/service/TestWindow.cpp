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
#include <service/WindowImpl.h>
#include <unit/service/Mocks.h>

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

class TestWindow: public Test {
protected:
	TestWindow() :
			mock(dbus) {
		factory.setSessionBus(dbus.sessionConnection());
	}

	virtual ~TestWindow() {
	}

	DBusTestRunner dbus;

	DBusMock mock;

	NiceMock<MockFactory> factory;
};

TEST_F(TestWindow, TrysDBusMenu) {
	QSharedPointer<MockCollector> dbusMenuCollector(
			new NiceMock<MockCollector>());
	ON_CALL(*dbusMenuCollector, isValid()).WillByDefault(Return(true));
	EXPECT_CALL(*dbusMenuCollector, collect());

	EXPECT_CALL(factory, newDBusMenuCollector(1234, QString("application-id"))).Times(
			1).WillOnce(Return(dbusMenuCollector));

	WindowImpl window(1234, "application-id", factory);
}

TEST_F(TestWindow, TrysDBusMenuThenGMenu) {
	QSharedPointer<MockCollector> dbusMenuCollector(
			new NiceMock<MockCollector>());
	ON_CALL(*dbusMenuCollector, isValid()).WillByDefault(Return(false));

	QSharedPointer<MockCollector> gmenuCollector(new NiceMock<MockCollector>());
	ON_CALL(*gmenuCollector, isValid()).WillByDefault(Return(true));
	EXPECT_CALL(*gmenuCollector, collect());

	EXPECT_CALL(factory, newDBusMenuCollector(1234, QString("application-id"))).Times(
			1).WillOnce(Return(dbusMenuCollector));
	EXPECT_CALL(factory, newGMenuCollector(1234, QString("application-id"))).Times(
			1).WillOnce(Return(gmenuCollector));

	WindowImpl window(1234, "application-id", factory);
}

} // namespace
