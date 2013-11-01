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

#include <common/AppstackModel.h>
#include <common/DBusTypes.h>
#include <common/Localisation.h>
#include <common/ResultsModel.h>
#include <service/HudService.h>
#include <service/QueryImpl.h>
#include <service/QueryAdaptor.h>

#include <QStringList>

using namespace hud::common;
using namespace hud::service;

QueryImpl::QueryImpl(unsigned int id, const QString &query, HudService &service,
		const QDBusConnection &connection, QObject *parent) :
		Query(parent), m_adaptor(new QueryAdaptor(this)), m_connection(
				connection), m_path(DBusTypes::queryPath(id)), m_service(
				service), m_query(query) {
	if (!m_connection.registerObject(m_path.path(), this)) {
		throw std::logic_error(
				_("Unable to register HUD query object on DBus"));
	}

	m_resultsModel.reset(new ResultsModel(id));
	m_appstackModel.reset(new AppstackModel(id));

	qDebug() << "Query constructed" << query << m_path.path();
}

QueryImpl::~QueryImpl() {
	m_connection.unregisterObject(m_path.path());

	qDebug() << "Query destroyed" << m_path.path();
}

const QDBusObjectPath & QueryImpl::path() const {
	return m_path;
}

QString QueryImpl::appstackModel() const {
	return QString::fromStdString(m_appstackModel->name());
}

QString QueryImpl::currentQuery() const {
	return m_query;
}

QString QueryImpl::resultsModel() const {
	return QString::fromStdString(m_appstackModel->name());
}

QStringList QueryImpl::toolbarItems() const {
	return QStringList();
}

void QueryImpl::CloseQuery() {
	m_service.closeQuery(m_path);
}

void QueryImpl::ExecuteCommand(const QDBusVariant &item, uint timestamp) {
}

QString QueryImpl::ExecuteParameterized(const QDBusVariant &item,
		uint timestamp, QDBusObjectPath &actionPath, QDBusObjectPath &modelPath,
		int &modelSection) {
	return QString();
}

void QueryImpl::ExecuteToolbar(const QString &item, uint timestamp) {

}

int QueryImpl::UpdateApp(const QString &app) {
	return 0;
}

int QueryImpl::UpdateQuery(const QString &query) {
	m_query = query;
	qDebug() << "query updated to" << query;
	return 0;
}

int QueryImpl::VoiceQuery(QString &query) {
	query = QString("no voice yet");
	return 0;
}
