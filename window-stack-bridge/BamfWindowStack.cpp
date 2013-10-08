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

#include <BamfWindowStack.h>
#include <Localisation.h>

#include <QFile>
#include <QFileInfo>

static const QString BAMF_DBUS_NAME("org.ayatana.bamf");

BamfWindow::BamfWindow(const QString &path, const QDBusConnection &connection) :
		m_window(BAMF_DBUS_NAME, path, connection), m_view(BAMF_DBUS_NAME, path,
				connection), m_windowId(m_window.GetXid()) {

	QDBusConnectionInterface* interface = connection.interface();
	if (!interface->isServiceRegistered(BAMF_DBUS_NAME)) {
		QDBusReply<void> reply(interface->startService(BAMF_DBUS_NAME));
	}

	QStringList parents(m_view.Parents());

	if (parents.empty()) {
		qWarning() << _("Window: ") << windowId() << m_view.name()
				<< _("has no parents");
	} else {
		OrgAyatanaBamfApplicationInterface application(BAMF_DBUS_NAME,
				parents.first(), m_window.connection());
		QFile desktopFile(application.DesktopFile());
		if (desktopFile.exists()) {
			m_applicationId = QFileInfo(desktopFile).baseName();
		}
	}

	if (m_applicationId.isEmpty()) {
		m_applicationId = QString::number(m_windowId);
	}
}

BamfWindow::~BamfWindow() {
}

unsigned int BamfWindow::windowId() {
	return m_windowId;
}

const QString & BamfWindow::applicationId() {

	return m_applicationId;
}

const QString BamfWindow::xProp(const QString &property) {
	return m_window.Xprop(property);
}

BamfWindowStack::WindowPtr BamfWindowStack::addWindow(const QString& path) {
	WindowPtr window(new BamfWindow(path, m_connection));
	m_windows[path] = window;
	m_windowsById[window->windowId()] = window;
	return window;
}

BamfWindowStack::WindowPtr BamfWindowStack::removeWindow(const QString& path) {
	WindowPtr window(m_windows.take(path));
	m_windowsById.remove(window->windowId());
	return window;
}

BamfWindowStack::BamfWindowStack(const QDBusConnection &connection,
		QObject *parent) :
		AbstractWindowStack(connection, parent), m_matcher(BAMF_DBUS_NAME,
				"/org/ayatana/bamf/matcher", connection) {

	registerOnBus();

	connect(&m_matcher,
	SIGNAL(ActiveWindowChanged(const QString&, const QString&)), this,
	SLOT(ActiveWindowChanged(const QString&, const QString&)));

	connect(&m_matcher,
	SIGNAL(ViewClosed(const QString&, const QString&)), this,
	SLOT(ViewClosed(const QString&, const QString&)));

	connect(&m_matcher,
	SIGNAL(ViewOpened(const QString&, const QString&)), this,
	SLOT(ViewOpened(const QString&, const QString&)));

	QStringList windowPaths(m_matcher.WindowPaths());
	for (const QString &path : windowPaths) {
		addWindow(path);
	}
}

BamfWindowStack::~BamfWindowStack() {
}

QString BamfWindowStack::GetAppIdFromPid(uint pid) {
	// FIXME Not implemented
	sendErrorReply(QDBusError::NotSupported,
			"GetAppIdFromPid method not implemented");
	return QString();
}

QList<WindowInfo> BamfWindowStack::GetWindowStack() {
	QList<WindowInfo> results;

	QStringList stack(m_matcher.WindowStackForMonitor(-1));
	for (const QString &path : stack) {
		WindowPtr window(m_windows[path]);
		results
				<< WindowInfo(window->windowId(), window->applicationId(),
						false);
	}

	WindowPtr window(m_windows[m_matcher.ActiveWindow()]);
	if (!window.isNull()) {
		uint windowId(window->windowId());

		for (WindowInfo &windowInfo : results) {
			if (windowInfo.window_id == windowId) {
				windowInfo.focused = true;
			}
		}
	}

	return results;
}

QString BamfWindowStack::GetWindowProperty(uint windowId, const QString &appId,
		const QString &name) {
	return m_windowsById[windowId]->xProp(name);
}

void BamfWindowStack::ActiveWindowChanged(const QString &oldWindowPath,
		const QString &newWindowPath) {
	if (!newWindowPath.isEmpty()) {
		WindowPtr window(m_windows[newWindowPath]);
		FocusedWindowChanged(window->windowId(), window->applicationId(),
				WindowInfo::MAIN);
	}
}

void BamfWindowStack::ViewClosed(const QString &path, const QString &type) {
	if (type == "window") {
		WindowPtr window(removeWindow(path));
		WindowDestroyed(window->windowId(), window->applicationId());
	}
}

void BamfWindowStack::ViewOpened(const QString &path, const QString &type) {
	if (type == "window") {
		WindowPtr window(addWindow(path));
		WindowCreated(window->windowId(), window->applicationId());
	}
}
