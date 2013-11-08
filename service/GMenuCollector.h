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

#ifndef HUD_SERVICE_GMENUCOLLECTOR_H_
#define HUD_SERVICE_GMENUCOLLECTOR_H_

#include <service/Collector.h>

class ComCanonicalUnityWindowStackInterface;

namespace hud {
namespace service {

class GMenuCollector: public Collector {
public:
	typedef QSharedPointer<GMenuCollector> Ptr;

	explicit GMenuCollector(unsigned int windowId, const QString &applicationId,
			QSharedPointer<ComCanonicalUnityWindowStackInterface> windowStack);

	virtual ~GMenuCollector();

	virtual bool isValid() const;

	virtual CollectorToken::Ptr activate();

	virtual void search(const QString &query, QList<Result> &results);

protected:
	virtual void deactivate();

	QSharedPointer<ComCanonicalUnityWindowStackInterface> m_windowStack;

	bool m_valid;

	QString m_busName;

	QDBusObjectPath m_appmenuPath;

	QDBusObjectPath m_menubarPath;

	QDBusObjectPath m_applicationPath;

	QDBusObjectPath m_windowPath;

	QDBusObjectPath m_unityPath;
};

}
}

#endif /* HUD_SERVICE_GMENUCOLLECTOR_H_ */
