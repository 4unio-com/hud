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
		QSharedPointer<ComCanonicalUnityWindowStackInterface> windowStack,
		const QDBusConnection &connection) :
		m_windowStack(windowStack), m_factory(factory), m_applicationCounter(0) {

	QDBusPendingReply<QList<WindowInfo>> windowsReply(
			m_windowStack->GetWindowStack());

	if (windowsReply.isError()) {
		qWarning() << windowsReply.error();
		return;
	}

	QList<WindowInfo> windows(windowsReply);
	for (const WindowInfo &window : windows) {
		Application::Ptr application(ensureApplication(window.app_id));
		application->addWindow(window.window_id);
	}

	connect(m_windowStack.data(),
	SIGNAL(FocusedWindowChanged(uint, const QString &, uint)), this,
	SLOT(FocusedWindowChanged(uint, const QString &, uint)));

	connect(m_windowStack.data(),
	SIGNAL(WindowCreated(uint, const QString &)), this,
	SLOT(WindowCreated(uint, const QString &)));

	connect(m_windowStack.data(),
	SIGNAL(WindowDestroyed(uint, const QString &)), this,
	SLOT(WindowDestroyed(uint, const QString &)));
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

void ApplicationList::removeWindow(uint windowId,
		const QString& applicationId) {
	Application::Ptr application(m_applications[applicationId]);

	if (application.isNull()) {
		qWarning() << "Attempt to remove window" << windowId
				<< "from non-existent application" << applicationId;
		return;
	}

	application->removeWindow(windowId);

	// If the application has no windows left, then the best
	// we can do is assume it has been closed.
	if (application->isEmpty()) {
		m_applications.remove(applicationId);
	}
}

void ApplicationList::FocusedWindowChanged(uint windowId,
		const QString &applicationId, uint stage) {
	qDebug() << "FocusedWindowChanged" << windowId << applicationId;
}

void ApplicationList::WindowCreated(uint windowId,
		const QString &applicationId) {
	Application::Ptr application(ensureApplication(applicationId));
	application->addWindow(windowId);
}

void ApplicationList::WindowDestroyed(uint windowId,
		const QString &applicationId) {
	removeWindow(windowId, applicationId);
}
