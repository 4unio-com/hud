/*
 * MockHudService.h
 *
 *  Created on: 14 Nov 2013
 *      Author: pete
 */

#ifndef HUD_TESTUTILS_MOCKHUDSERVICE_H_
#define HUD_TESTUTILS_MOCKHUDSERVICE_H_

#include <libqtdbustest/DBusTestRunner.h>
#include <libqtdbusmock/DBusMock.h>

#include <QScopedPointer>

namespace hud {
namespace common {

class AppstackModel;
class ResultsModel;

}

namespace testutils {

class Q_DECL_EXPORT MockHudService {
public:
	static const QString QUERY_PATH;

	MockHudService(QtDBusTest::DBusTestRunner &dbus,
			QtDBusMock::DBusMock &mock);

	virtual ~MockHudService();

	void loadMethods();

	OrgFreedesktopDBusMockInterface & hudInterface();

	OrgFreedesktopDBusMockInterface & applicationInterface();

	OrgFreedesktopDBusMockInterface & queryInterface();

protected:
	QtDBusTest::DBusTestRunner &m_dbus;

	QtDBusMock::DBusMock &m_mock;

	QScopedPointer<hud::common::AppstackModel> m_appstack;

	QScopedPointer<hud::common::ResultsModel> m_results;

	QSharedPointer<OrgFreedesktopDBusMockInterface> m_applicationInterface;

	QSharedPointer<OrgFreedesktopDBusMockInterface> m_queryInterface;
};

}
}
#endif /* HUD_TESTUTILS_MOCKHUDSERVICE_H_ */
