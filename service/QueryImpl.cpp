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

QueryImpl::QueryImpl(unsigned int id, const QString &query,
		const QString &sender, HudService &service,
		ApplicationList::Ptr applicationList, const QDBusConnection &connection,
		QObject *parent) :
		Query(parent), m_adaptor(new QueryAdaptor(this)), m_connection(
				connection), m_path(DBusTypes::queryPath(id)), m_service(
				service), m_applicationList(applicationList), m_query(query), m_serviceWatcher(
				sender, m_connection,
				QDBusServiceWatcher::WatchForUnregistration) {

	connect(&m_serviceWatcher, SIGNAL(serviceUnregistered(const QString &)),
			this, SLOT(serviceUnregistered(const QString &)));

	m_resultsModel.reset(new ResultsModel(id));
	m_appstackModel.reset(new AppstackModel(id));

	refresh();

	if (!m_connection.registerObject(m_path.path(), this)) {
		throw std::logic_error(
				_("Unable to register HUD query object on DBus"));
	}
}

QueryImpl::~QueryImpl() {
	m_connection.unregisterObject(m_path.path());
}

const QDBusObjectPath & QueryImpl::path() const {
	return m_path;
}

const QList<Result> & QueryImpl::results() const {
	return m_results;
}

QString QueryImpl::appstackModel() const {
	return QString::fromStdString(m_appstackModel->name());
}

QString QueryImpl::currentQuery() const {
	return m_query;
}

QString QueryImpl::resultsModel() const {
	return QString::fromStdString(m_resultsModel->name());
}

QStringList QueryImpl::toolbarItems() const {
	//TODO Implement toolbarItems
	return QStringList();
}

void QueryImpl::CloseQuery() {
	m_service.closeQuery(m_path);
}

void QueryImpl::ExecuteCommand(const QDBusVariant &item, uint timestamp) {
	qDebug() << "ExecuteCommand" << item.variant() << timestamp;
}

QString QueryImpl::ExecuteParameterized(const QDBusVariant &item,
		uint timestamp, QDBusObjectPath &actionPath, QDBusObjectPath &modelPath,
		int &modelSection) {
	qDebug() << "ExecuteParameterized";
	return QString();
}

void QueryImpl::ExecuteToolbar(const QString &item, uint timestamp) {
	qDebug() << "ExecuteToolbar" << item << timestamp;
}

/**
 * This means that the user has clicked on an application
 * in the HUD user interface.
 */
int QueryImpl::UpdateApp(const QString &app) {
	qDebug() << "UpdateApp" << app;
	return 0;
}

int QueryImpl::UpdateQuery(const QString &query) {
	qDebug() << "UpdateQuery" << query;
	if (m_query == query) {
		return 0;
	}

	m_query = query;
	refresh();

	return 0;
}

void QueryImpl::refresh() {
	Application::Ptr application(m_applicationList->focusedApplication());
	if (application.isNull()) {
		qWarning() << "No focused application during query refresh";
		return;
	}

	Window::Ptr window(m_applicationList->focusedWindow());

	m_windowToken = window->activate();

	m_results.clear();
	m_windowToken->search(m_query, m_results);

	// Convert to results list to Dee model
	m_resultsModel->beginChangeset();
	for (const Result &result : m_results) {
		m_resultsModel->addResult(result.id(), result.commandName(),
				result.commandHighlights(), result.description(),
				result.descriptionHighlights(), result.shortcut(),
				result.distance(), result.parameterized());
	}
	m_resultsModel->endChangeset();

	m_appstackModel->beginChangeset();
	m_appstackModel->addApplication(application->id(), application->icon(),
			AppstackModel::ITEM_TYPE_FOCUSED_APP);
	//TODO Apps other than the foreground one
	m_appstackModel->endChangeset();
}

int QueryImpl::VoiceQuery(QString &query) {
	qDebug() << "VoiceQuery";

	query = QString("voice query");
	UpdateQuery(query);
	return 0;
}

void QueryImpl::serviceUnregistered(const QString &service) {
	Q_UNUSED(service);
	CloseQuery();
}
