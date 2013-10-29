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
#include <service/Application.h>
#include <service/ApplicationAdaptor.h>
#include <service/Factory.h>

using namespace hud::common;
using namespace hud::service;

Application::Application(unsigned int id, const QString &applicationId,
		Factory &factory, const QDBusConnection &connection, QObject *parent) :
		QObject(parent), m_adaptor(new ApplicationAdaptor(this)), m_connection(
				connection), m_path(DBusTypes::applicationPath(id)), m_applicationId(
				applicationId), m_factory(factory) {
	qDebug() << "new application" << m_applicationId;
	if (!m_connection.registerObject(m_path.path(), this)) {
		throw std::logic_error(_("Unable to register HUD object on DBus"));
	}
}

Application::~Application() {
	m_connection.unregisterObject(m_path.path());
}

void Application::addWindow(unsigned int windowId) {
	if (m_windows.contains(windowId)) {
		qWarning() << "Adding already known window" << windowId
				<< "to application" << m_applicationId;
		return;
	}
	qDebug() << "adding window" << windowId;
	m_windows[windowId] = m_factory.newWindow(windowId, m_applicationId);
}

void Application::removeWindow(unsigned int windowId) {
	if (!m_windows.contains(windowId)) {
		qWarning() << "Removing unknown window" << windowId
				<< "from application" << m_applicationId;
		return;
	}
	qDebug() << "removing window" << windowId;
	m_windows.remove(windowId);
}

bool Application::isEmpty() const {
	return m_windows.isEmpty();
}

QList<ActionGroup> Application::actionGroups() const {
	return QList<ActionGroup>();
}

QString Application::desktopPath() const {
	return QString();
}

QString Application::icon() const {
	return QString();
}

QList<MenuModel> Application::menuModels() const {
	return QList<MenuModel>();
}

void Application::AddSources(const QList<Action> &actions,
		const QList<Description> &descriptions) {
}

void Application::SetWindowContext(uint window, const QString &context) {
}
