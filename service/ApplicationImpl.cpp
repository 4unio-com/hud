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
#include <service/ApplicationImpl.h>
#include <service/ApplicationAdaptor.h>
#include <service/Factory.h>

using namespace hud::common;
using namespace hud::service;

ApplicationImpl::ApplicationImpl(unsigned int id, const QString &applicationId,
		Factory &factory, const QDBusConnection &connection, QObject *parent) :
		Application(parent), m_adaptor(new ApplicationAdaptor(this)), m_connection(
				connection), m_path(DBusTypes::applicationPath(id)), m_applicationId(
				applicationId), m_factory(factory) {
	qDebug() << "new application" << m_applicationId;
	if (!m_connection.registerObject(m_path.path(), this)) {
		throw std::logic_error(_("Unable to register HUD object on DBus"));
	}
}

ApplicationImpl::~ApplicationImpl() {
	m_connection.unregisterObject(m_path.path());
}

void ApplicationImpl::addWindow(unsigned int windowId) {
	if (m_windows.contains(windowId)) {
		qWarning() << "Adding already known window" << windowId
				<< "to application" << m_applicationId;
		return;
	}
	qDebug() << "adding window" << windowId;
	m_windows[windowId] = m_factory.newWindow(windowId, m_applicationId);
}

void ApplicationImpl::removeWindow(unsigned int windowId) {
	if (!m_windows.contains(windowId)) {
		qWarning() << "Removing unknown window" << windowId
				<< "from application" << m_applicationId;
		return;
	}
	qDebug() << "removing window" << windowId;
	m_windows.remove(windowId);
}

bool ApplicationImpl::isEmpty() const {
	return m_windows.isEmpty();
}

const QDBusObjectPath & ApplicationImpl::path() const {
	return m_path;
}

QList<ActionGroup> ApplicationImpl::actionGroups() const {
	return QList<ActionGroup>();
}

QString ApplicationImpl::desktopPath() const {
	return QString();
}

QString ApplicationImpl::icon() const {
	return QString();
}

QList<MenuModel> ApplicationImpl::menuModels() const {
	return QList<MenuModel>();
}

void ApplicationImpl::AddSources(const QList<Action> &actions,
		const QList<Description> &descriptions) {
}

void ApplicationImpl::SetWindowContext(uint window, const QString &context) {
}
