/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Pete Woods <pete.woods@canonical.com>
 */

#ifndef HUD_SERVICE_TEST_MOCKS_H_
#define HUD_SERVICE_TEST_MOCKS_H_

#include <service/Factory.h>
#include <service/ApplicationList.h>

#include <gmock/gmock.h>

namespace hud {
namespace service {
namespace test {

class MockFactory: public Factory {
public:
	MOCK_METHOD0(newApplicationList, ApplicationList::Ptr());

	MOCK_METHOD0(sessionBus, QDBusConnection());

	MOCK_METHOD2(newQuery, Query::Ptr(unsigned int, const QString &));

	MOCK_METHOD2(newApplication, Application::Ptr(unsigned int, const QString &));

	MOCK_METHOD2(newWindow, Window::Ptr(unsigned int, const QString &));

	MOCK_METHOD2(newDBusMenuCollector, Collector::Ptr(unsigned int, const QString &));

	MOCK_METHOD2(newGMenuCollector, Collector::Ptr(unsigned int, const QString &));
};

class MockQuery: public Query {
public:
	MOCK_CONST_METHOD0(appstackModel, QString());

	MOCK_CONST_METHOD0(currentQuery, QString());

	MOCK_CONST_METHOD0(resultsModel, QString());

	MOCK_CONST_METHOD0(toolbarItems, QStringList());

	MOCK_CONST_METHOD0(path, const QDBusObjectPath &());

	MOCK_CONST_METHOD0(results, const QList<Result> &());

	MOCK_METHOD1(UpdateQuery, int(const QString &));
};

class MockApplicationList: public ApplicationList {
public:
	MOCK_CONST_METHOD0(applications, QList<hud::common::NameObject>());
};

class MockApplication: public Application {
public:
	MOCK_METHOD1(addWindow, void(unsigned int));

	MOCK_METHOD1(removeWindow, void(unsigned int));

	MOCK_CONST_METHOD0(isEmpty, bool());

	MOCK_CONST_METHOD0(path, const QDBusObjectPath &());
};

class MockWindow: public Window {
public:
};

class MockCollector: public Collector {
public:
	MOCK_CONST_METHOD0(isValid, bool());

	MOCK_METHOD0(collect, void());
};

}
}
}

#endif /* HUD_SERVICE_TEST_MOCKS_H_ */
