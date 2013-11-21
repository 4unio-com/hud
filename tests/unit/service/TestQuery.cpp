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
#include <service/QueryImpl.h>
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

class TestQuery: public Test {
protected:
	TestQuery() :
			mock(dbus) {

//		mock.registerCustomMock("keep.alive", "/", "keep.alive",
//				QDBusConnection::SessionBus);
//
//		dbus.startServices();
	}

	virtual ~TestQuery() {
	}

	DBusTestRunner dbus;

	DBusMock mock;
};

TEST_F(TestQuery, Foo) {
	NiceMock<MockHudService> hudService;

	QList<Result> expectedResults;
	expectedResults
			<< Result(0, "command name",
					Result::HighlightList() << Result::Highlight(0, 1),
					"description field",
					Result::HighlightList() << Result::Highlight(2, 3),
					"shortcut field", 100, false);
	QString queryString("query");

	QSharedPointer<MockWindowToken> windowToken(
			new NiceMock<MockWindowToken>());
	EXPECT_CALL(*windowToken, search(queryString, _)).WillOnce(
			Invoke(
					[&expectedResults](const QString &query, QList<Result> &results) {
						results.append(expectedResults);
					}));

	QSharedPointer<MockApplication> application(
			new NiceMock<MockApplication>());
	QString id("app-id");
	QString icon("app-icon");
	ON_CALL(*application, id()).WillByDefault(ReturnRef(id));
	ON_CALL(*application, icon()).WillByDefault(ReturnRef(icon));

	QSharedPointer<MockWindow> window(new NiceMock<MockWindow>());
	EXPECT_CALL(*window, activate()).WillOnce(Return(windowToken));

	QSharedPointer<MockApplicationList> applicationList(
			new NiceMock<MockApplicationList>());
	ON_CALL(*applicationList, focusedApplication()).WillByDefault(
			Return(application));
	ON_CALL(*applicationList, focusedWindow()).WillByDefault(Return(window));

	QueryImpl query(0, queryString, "keep.alive", hudService, applicationList,
			dbus.sessionConnection());

	const QList<Result> results(query.results());
	ASSERT_EQ(expectedResults.size(), results.size());
	ASSERT_EQ(expectedResults.at(0).id(), results.at(0).id());
	ASSERT_EQ(expectedResults.at(0).commandName(), results.at(0).commandName());
	ASSERT_EQ(expectedResults.at(0).description(), results.at(0).description());
}

} // namespace
