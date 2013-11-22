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
	if (!m_connection.registerObject(m_path.path(), this)) {
		throw std::logic_error(_("Unable to register HUD object on DBus"));
	}

	m_allWindowsContext = m_factory.newWindowContext();
}

ApplicationImpl::~ApplicationImpl() {
	m_connection.unregisterObject(m_path.path());
}

const QString & ApplicationImpl::id() const {
	return m_applicationId;
}

void ApplicationImpl::addWindow(unsigned int windowId) {
	if (m_windows.contains(windowId)) {
		qWarning() << "Adding already known window" << windowId
				<< "to application" << m_applicationId;
		return;
	}
	m_windows[windowId] = m_factory.newWindow(windowId, m_applicationId,
			m_allWindowsContext);
}

void ApplicationImpl::removeWindow(unsigned int windowId) {
	if (!m_windows.contains(windowId)) {
		qWarning() << "Removing unknown window" << windowId
				<< "from application" << m_applicationId;
		return;
	}
	m_windows.remove(windowId);
}

Window::Ptr ApplicationImpl::window(unsigned int windowId) {
	return m_windows[windowId];
}

WindowContext::Ptr ApplicationImpl::windowContext(unsigned int windowId) {
	if (windowId == WINDOW_ID_ALL_WINDOWS) {
		return m_allWindowsContext;
	}
	return m_windows[windowId];
}

bool ApplicationImpl::isEmpty() const {
	return m_windows.isEmpty();
}

const QDBusObjectPath & ApplicationImpl::path() const {
	return m_path;
}

const QString & ApplicationImpl::desktopPath() {
	if (m_desktopPath.isEmpty()) {
		QString desktopFile(QString("%1.desktop").arg(m_applicationId));

		QStringList xdgDataDirs(
				QString::fromUtf8(qgetenv("XDG_DATA_DIRS")).split(':'));
		for (const QString &dir : xdgDataDirs) {
			QString desktopPath(
					QDir(QDir(dir).filePath("applications")).filePath(
							desktopFile));
			if (QFile::exists(desktopPath)) {
				m_desktopPath = desktopPath;
				break;
			}
		}
	}

	return m_desktopPath;
}

const QString & ApplicationImpl::icon() {
	if (m_icon.isEmpty()) {
		QString path(desktopPath());
		if (!path.isEmpty()) {
			QSettings settings(path, QSettings::IniFormat);
			settings.beginGroup("Desktop Entry");
			m_icon = settings.value("Icon").toString();
			settings.endGroup();
		}
	}

	return m_icon;
}

QList<ActionGroup> ApplicationImpl::actionGroups() const {
	qDebug() << "actionGroups";
	return QList<ActionGroup>();
}

QList<MenuModel> ApplicationImpl::menuModels() const {
	qDebug() << "menuModels";
	return QList<MenuModel>();
}

QString ApplicationImpl::messageSender() {
	QString sender("local");
	if (calledFromDBus()) {
		sender = message().service();
	}
	return sender;
}

/**
 * Window ID 0 is the "all windows" context
 */
void ApplicationImpl::AddSources(const QList<Action> &actions,
		const QList<Description> &descriptions) {
	QString name(messageSender());

	for (const Action &action : actions) {
		WindowContext::Ptr window = windowContext(action.m_windowId);

		if (window.isNull()) {
			qWarning()
					<< "Tried to add action source for unknown window context"
					<< action.m_windowId;
			continue;
		}

		window->addAction(action.m_context, name, action.m_object,
				action.m_prefix);
	}

	for (const Description &description : descriptions) {
		WindowContext::Ptr window = windowContext(description.m_windowId);

		if (window.isNull()) {
			qWarning() << "Tried to add model source for unknown window context"
					<< description.m_windowId;
			continue;
		}

		window->addModel(description.m_context, name, description.m_object);
	}
}

void ApplicationImpl::SetWindowContext(uint windowId, const QString &context) {
	WindowContext::Ptr window = windowContext(windowId);
	if (window.isNull()) {
		qWarning() << "Tried to set context on unknown window context"
				<< windowId << m_applicationId;
		return;
	}
	window->setContext(context);
}
