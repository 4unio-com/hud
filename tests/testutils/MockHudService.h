/*
 * MockHudService.h
 *
 *  Created on: 14 Nov 2013
 *      Author: pete
 */

#ifndef HUD_TESTUTILS_MOCKHUDSERVICE_H_
#define HUD_TESTUTILS_MOCKHUDSERVICE_H_

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
	MockHudService(QtDBusMock::DBusMock &mock);

	virtual ~MockHudService();

	void loadMethods();

protected:
	QtDBusMock::DBusMock &m_mock;

	QScopedPointer<hud::common::AppstackModel> m_appstack;

	QScopedPointer<hud::common::ResultsModel> m_results;
};

}
}
#endif /* HUD_TESTUTILS_MOCKHUDSERVICE_H_ */
