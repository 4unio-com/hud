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
#include <service/HudService.h>

#include <QDebug>
#include <libqtdbustest/DBusTestRunner.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace std;
using namespace testing;
using namespace QtDBusTest;
using namespace hud::service;

namespace {

class MockFactory: public Factory {
public:
	MOCK_METHOD0(newApplicationList, ApplicationList::Ptr());

	MOCK_METHOD2(newQuery, Query::Ptr(unsigned int, const QString &));
};

class MockQuery: public Query {
public:
	MOCK_CONST_METHOD0(appstackModel, QString());
	MOCK_CONST_METHOD0(currentQuery, QString());
	MOCK_CONST_METHOD0(resultsModel, QString());
	MOCK_CONST_METHOD0(toolbarItems, QStringList());
	MOCK_CONST_METHOD0(path, const QDBusObjectPath &());
};

class TestHudService: public Test {
protected:
	TestHudService() {
		ON_CALL(factory, newApplicationList()).WillByDefault(
				Return(ApplicationList::Ptr(new ApplicationList())));
	}

	virtual ~TestHudService() {
	}

	DBusTestRunner dbus;

	NiceMock<MockFactory> factory;
};

TEST_F(TestHudService, OpenCloseQuery) {
	HudService hudService(factory, dbus.sessionConnection());

	QDBusObjectPath queryPath("/path/query0");
	QString resultsModel("com.canonical.hud.results0");
	QString appstackModel("com.canonical.hud.appstack0");
	QSharedPointer<MockQuery> query(new NiceMock<MockQuery>());
	ON_CALL(*query, path()).WillByDefault(ReturnRef(queryPath));
	ON_CALL(*query, resultsModel()).WillByDefault(Return(resultsModel));
	ON_CALL(*query, appstackModel()).WillByDefault(Return(appstackModel));

	EXPECT_CALL(factory, newQuery(0, QString("query text"))).Times(1).WillOnce(
			Return(query));

	QString resultsName;
	QString appstackName;
	int modelRevision;

	EXPECT_EQ(queryPath,
			hudService.CreateQuery("query text", resultsName, appstackName,
					modelRevision));
	EXPECT_EQ(resultsModel, resultsName);
	EXPECT_EQ(appstackModel, appstackName);
	EXPECT_EQ(0, modelRevision);

	EXPECT_EQ(QList<QDBusObjectPath>() << queryPath, hudService.openQueries());
	hudService.closeQuery(queryPath);
	EXPECT_EQ(QList<QDBusObjectPath>(), hudService.openQueries());
}

TEST_F(TestHudService, CloseUnknownQuery) {
	HudService hudService(factory, dbus.sessionConnection());

	QDBusObjectPath queryPath("/path/query0");

	EXPECT_EQ(QList<QDBusObjectPath>(), hudService.openQueries());
	hudService.closeQuery(queryPath);
	EXPECT_EQ(QList<QDBusObjectPath>(), hudService.openQueries());
}

TEST_F(TestHudService, CreateMultipleQueries) {
	HudService hudService(factory, dbus.sessionConnection());

	QDBusObjectPath queryPath0("/path/query0");
	QString resultsModel0("com.canonical.hud.results0");
	QString appstackModel0("com.canonical.hud.appstack0");
	QSharedPointer<MockQuery> query0(new NiceMock<MockQuery>());
	ON_CALL(*query0, path()).WillByDefault(ReturnRef(queryPath0));
	ON_CALL(*query0, resultsModel()).WillByDefault(Return(resultsModel0));
	ON_CALL(*query0, appstackModel()).WillByDefault(Return(appstackModel0));

	QDBusObjectPath queryPath1("/path/query1");
	QString resultsModel1("com.canonical.hud.results1");
	QString appstackModel1("com.canonical.hud.appstack1");
	QSharedPointer<MockQuery> query1(new NiceMock<MockQuery>());
	ON_CALL(*query1, path()).WillByDefault(ReturnRef(queryPath1));
	ON_CALL(*query1, resultsModel()).WillByDefault(Return(resultsModel1));
	ON_CALL(*query1, appstackModel()).WillByDefault(Return(appstackModel1));

	EXPECT_CALL(factory, newQuery(0, QString("query0"))).Times(1).WillOnce(
			Return(query0));
	EXPECT_CALL(factory, newQuery(1, QString("query1"))).Times(1).WillOnce(
			Return(query1));

	int modelRevision;
	QString resultsName;
	QString appstackName;

	EXPECT_EQ(queryPath0,
			hudService.CreateQuery("query0", resultsName, appstackName,
					modelRevision));
	EXPECT_EQ(resultsModel0, resultsName);
	EXPECT_EQ(appstackModel0, appstackName);
	EXPECT_EQ(0, modelRevision);
	EXPECT_EQ(QList<QDBusObjectPath>() << queryPath0, hudService.openQueries());

	EXPECT_EQ(queryPath1,
			hudService.CreateQuery("query1", resultsName, appstackName,
					modelRevision));
	EXPECT_EQ(resultsModel1, resultsName);
	EXPECT_EQ(appstackModel1, appstackName);
	EXPECT_EQ(0, modelRevision);
	EXPECT_EQ(QList<QDBusObjectPath>() << queryPath0 << queryPath1,
			hudService.openQueries());

	hudService.closeQuery(queryPath0);
	EXPECT_EQ(QList<QDBusObjectPath>() << queryPath1, hudService.openQueries());

	hudService.closeQuery(queryPath1);
	EXPECT_EQ(QList<QDBusObjectPath>(), hudService.openQueries());
}

} // namespace
