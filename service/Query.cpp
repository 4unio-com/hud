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

#include <common/DBusTypes.h>
#include <common/Localisation.h>
#include <service/HudService.h>
#include <service/Query.h>
#include <service/QueryAdaptor.h>

#include <QStringList>

using namespace hud::common;

namespace hud {
namespace service {

Query::Query(unsigned int id, const QString &query, HudService &service,
		const QDBusConnection &connection, QObject *parent) :
		QObject(parent), m_adaptor(new QueryAdaptor(this)), m_connection(
				connection), m_path(DBusTypes::queryPath(id)), m_service(
				service), m_query(query) {
	if (!m_connection.registerObject(m_path.path(), this)) {
		throw std::logic_error(
				_("Unable to register HUD query object on DBus"));
	}
}

Query::~Query() {
	m_connection.unregisterObject(m_path.path());
}

const QDBusObjectPath & Query::path() const {
	return m_path;
}

QString Query::appstackModel() const {
	return QString();
}

QString Query::currentQuery() const {
	return m_query;
}

QString Query::resultsModel() const {
	return QString();
}

QStringList Query::toolbarItems() const {
	return QStringList();
}

void Query::CloseQuery() {
	m_service.closeQuery(m_path);
}

void Query::ExecuteCommand(const QDBusVariant &item, uint timestamp) {
}

QString Query::ExecuteParameterized(const QDBusVariant &item, uint timestamp,
		QDBusObjectPath &actionPath, QDBusObjectPath &modelPath,
		int &modelSection) {
	return QString();
}

void Query::ExecuteToolbar(const QString &item, uint timestamp) {

}

int Query::UpdateApp(const QString &app) {
	return 0;
}

int Query::UpdateQuery(const QString &query) {
	m_query = query;
	return 0;
}

int Query::VoiceQuery(QString &query) {
	query = QString("no voice yet");
	return 0;
}

}
}
