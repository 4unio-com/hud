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
#include <common/WindowStackInterface.h>
#include <window-stack-bridge/PlatformApiWindowStack.h>

#include <libqtdbustest/DBusTestRunner.h>
#include <libqtdbusmock/DBusMock.h>
#include <QSignalSpy>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace std;
using namespace testing;
using namespace hud::common;
using namespace QtDBusTest;
using namespace QtDBusMock;

namespace {

class TestPlatformApiWindowStack: public Test {
protected:
	TestPlatformApiWindowStack() :
			mock(dbus) {
	}

	virtual ~TestPlatformApiWindowStack() {
	}

	DBusTestRunner dbus;

	DBusMock mock;
};

TEST_F(TestPlatformApiWindowStack, ExportsDBusInterface) {
}

} // namespace
