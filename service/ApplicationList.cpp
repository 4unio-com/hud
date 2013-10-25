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
#include <service/ApplicationList.h>
#include <service/Factory.h>

#include <QDebug>

using namespace hud::common;
using namespace hud::service;

ApplicationList::ApplicationList(Factory &factory,
		const QDBusConnection &connection) :
		m_windowStack(DBusTypes::WINDOW_STACK_DBUS_NAME,
				DBusTypes::WINDOW_STACK_DBUS_PATH, connection), m_factory(
				factory), m_applicationCounter(0) {
	QDBusPendingReply<QList<WindowInfo>> windowsReply(
			m_windowStack.GetWindowStack());

	if (windowsReply.isError()) {
		qWarning() << windowsReply.error();
		return;
	}

	QList<WindowInfo> windows(windowsReply);
	for (const WindowInfo &window : windows) {
		Application::Ptr application(ensureApplication(window.app_id));
		application->addWindow(window.window_id);
	}

	//TODO Listen to window updates
}

ApplicationList::~ApplicationList() {
}

Application::Ptr ApplicationList::ensureApplication(
		const QString& applicationId) {
	Application::Ptr application(m_applications[applicationId]);
	if (application.isNull()) {
		application = m_factory.newApplication(m_applicationCounter++,
				applicationId);
		m_applications[applicationId] = application;
	}
	return application;
}
