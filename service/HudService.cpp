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

#include <service/ApplicationList.h>
#include <service/Factory.h>
#include <service/HudService.h>
#include <service/HudAdaptor.h>
#include <common/Localisation.h>
#include <common/DBusTypes.h>

using namespace hud::common;

namespace hud {
namespace service {

HudService::HudService(Factory &factory, const QDBusConnection &connection,
		QObject *parent) :
		QObject(parent), m_adaptor(new HudAdaptor(this)), m_connection(
				connection), m_factory(factory) {
	DBusTypes::registerMetaTypes();

	m_applicationList = m_factory.newApplicationList();

	if (!m_connection.registerObject(DBusTypes::HUD_SERVICE_DBUS_PATH, this)) {
		throw std::logic_error(_("Unable to register HUD object on DBus"));
	}
	if (!m_connection.registerService(DBusTypes::HUD_SERVICE_DBUS_NAME)) {
		throw std::logic_error(_("Unable to register HUD service on DBus"));
	}
}

HudService::~HudService() {
	m_connection.unregisterObject(DBusTypes::HUD_SERVICE_DBUS_PATH);
}

QDBusObjectPath HudService::RegisterApplication(const QString &id) {
	return QDBusObjectPath("/");
}

QList<NameObject> HudService::applications() const {
	return m_applicationList->applications();
}

QList<QDBusObjectPath> HudService::openQueries() const {
	return m_queries.keys();
}

QDBusObjectPath HudService::CreateQuery(const QString &query,
		QString &resultsName, QString &appstackName, int &modelRevision) {

	Query::Ptr hudQuery(m_factory.newQuery(query));

	m_queries[hudQuery->path()] = hudQuery;

	resultsName = hudQuery->resultsModel();
	appstackName = hudQuery->appstackModel();
	modelRevision = 0;

	return QDBusObjectPath(hudQuery->path());
}

Query::Ptr HudService::closeQuery(const QDBusObjectPath &path) {
	return m_queries.take(path);
}

QString HudService::messageSender() {
	QString sender("local");
	if (calledFromDBus()) {
		sender = message().service();
	}
	return sender;
}

/*
 * Legacy interface below here
 */

QString HudService::StartQuery(const QString &queryString, int entries,
		QList<Suggestion> &suggestions, QDBusVariant &querykey) {

	QString sender(messageSender());

	QString resultsName;
	QString appstackName;
	int modelRevision;

	Query::Ptr query(m_legacyQueries[sender]);
	if (query.isNull()) {
		QDBusObjectPath path(
				CreateQuery(queryString, resultsName, appstackName,
						modelRevision));
		query = m_queries[path];
		m_legacyQueries[sender] = query;
	} else {
		query->UpdateQuery(queryString);
	}

	//FIXME Hard-coded icon value
	for (const Result &result : query->results()) {
		suggestions
				<< Suggestion(result.id(), result.commandName(),
						result.commandHighlights(), result.description(),
						result.descriptionHighlights(), QString("icon"));
	}
	querykey.setVariant(query->path().path());

	return queryString;
}

void HudService::ExecuteQuery(const QDBusVariant &itemKey, uint timestamp) {
	Q_UNUSED(timestamp);

	QString sender(messageSender());

	qDebug() << "ExecuteQuery" << sender << itemKey.variant() << timestamp;
}

void HudService::CloseQuery(const QDBusVariant &querykey) {
	Q_UNUSED(querykey);
	QString sender(messageSender());

	qDebug() << "CloseQuery";

	// We don't actually close legacy queries, or we'd be constructing
	// and destructing them during the search due to the way that
	// Unity7 uses the API.
	Query::Ptr query(m_legacyQueries[sender]);
	if (!query.isNull()) {
		query->UpdateQuery(QString());
	}
}

}
}
