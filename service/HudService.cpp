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
				connection), m_factory(factory), m_queryCounter(0) {
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

	Query::Ptr hudQuery(m_factory.newQuery(m_queryCounter++, query));

	m_queries[hudQuery->path()] = hudQuery;

	resultsName = hudQuery->resultsModel();
	appstackName = hudQuery->appstackModel();
	modelRevision = 0;

	return QDBusObjectPath(hudQuery->path());
}

void HudService::closeQuery(const QDBusObjectPath &path) {
	m_queries.remove(path);
}

/*
 * Legacy interface below here
 */

QString HudService::StartQuery(const QString &query, int entries,
		QList<Suggestion> &suggestions, QDBusVariant &querykey) {

	QString resultsName;
	QString appstackName;
	int modelRevision;

	QDBusObjectPath path(
			CreateQuery(query, resultsName, appstackName, modelRevision));

	suggestions = QList<Suggestion>();
	querykey = QDBusVariant(QVariant::fromValue(path));

	return query;
}

void HudService::ExecuteQuery(const QDBusVariant &key, uint timestamp) {
	QVariant variant(key.variant());
	if (!variant.canConvert<QDBusObjectPath>()) {
		sendErrorReply(QDBusError::InvalidArgs, _("Incorrect query key type"));
	}

}

void HudService::CloseQuery(const QDBusVariant &querykey) {
	QVariant variant(querykey.variant());
	if (!variant.canConvert<QDBusObjectPath>()) {
		sendErrorReply(QDBusError::InvalidArgs, _("Incorrect query key type"));
	}

	QDBusObjectPath path(variant.value<QDBusObjectPath>());
	closeQuery(path);
}

}
}
