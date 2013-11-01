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

#ifndef HUD_SERVICE_SERVICE_H_
#define HUD_SERVICE_SERVICE_H_

#include <common/DBusTypes.h>
#include <service/Query.h>

#include <QDBusContext>
#include <QMap>
#include <QScopedPointer>
#include <QSharedPointer>

class HudAdaptor;

namespace hud {
namespace service {

class ApplicationList;
class Factory;

class Q_DECL_EXPORT HudService: public QObject, protected QDBusContext {
Q_OBJECT

public:
	typedef QSharedPointer<HudService> Ptr;

	explicit HudService(Factory &factory, const QDBusConnection &connection,
			QObject *parent = 0);

	virtual ~HudService();

	Q_PROPERTY(QList<hud::common::NameObject> Applications READ applications)
	QList<hud::common::NameObject> applications() const;

	Q_PROPERTY(QList<QDBusObjectPath> OpenQueries READ openQueries)
	QList<QDBusObjectPath> openQueries() const;

	void closeQuery(const QDBusObjectPath &path);

public Q_SLOTS:
	QDBusObjectPath RegisterApplication(const QString &id);

	QDBusObjectPath CreateQuery(const QString &query, QString &resultsName,
			QString &appstackName, int &modelRevision);

	/*
	 * Legacy interface below here
	 */

	QString StartQuery(const QString &query, int entries,
			QList<hud::common::Suggestion> &suggestions,
			QDBusVariant &querykey);

	void ExecuteQuery(const QDBusVariant &key, uint timestamp);

	void CloseQuery(const QDBusVariant &querykey);

protected:
	QScopedPointer<HudAdaptor> m_adaptor;

	QDBusConnection m_connection;

	Factory &m_factory;

	unsigned int m_queryCounter;

	QMap<QDBusObjectPath, Query::Ptr> m_queries;

	QMap<QString, Query::Ptr> m_legacyQueries;

	QSharedPointer<ApplicationList> m_applicationList;
};

}
}

#endif /* HUD_SERVICE_SERVICE_H_ */
