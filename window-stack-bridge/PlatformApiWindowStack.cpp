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

#include <PlatformApiWindowStack.h>
#include <Localisation.h>

#include <QDebug>
#include <stdexcept>
#include <ubuntu/application/ui/stage.h>

PlatformApiWindowStack::PlatformApiWindowStack(
		const QDBusConnection &connection, QObject *parent) :
		AbstractWindowStack(connection, parent) {

	if (qgetenv("QT_QPA_PLATFORM") != "ubuntu") {
		throw std::logic_error(
				_("Incorrect QPA environment for Ubuntu platform API"));
	}

	m_observer_definition.on_session_requested = on_session_requested_cb;
	m_observer_definition.on_session_born = on_session_born_cb;
	m_observer_definition.on_session_unfocused = on_session_unfocused_cb;
	m_observer_definition.on_session_focused = on_session_focused_cb;
	m_observer_definition.on_session_died = on_session_died_cb;
	m_observer_definition.on_keyboard_geometry_changed = 0;
	m_observer_definition.on_session_requested_fullscreen = 0;
	m_observer_definition.context = reinterpret_cast<void *>(this);

	ubuntu_ui_session_install_session_lifecycle_observer(
			&m_observer_definition);
}

PlatformApiWindowStack::~PlatformApiWindowStack() {
}

QString PlatformApiWindowStack::GetAppIdFromPid(uint pid) {
	// FIXME Not implemented
	qDebug() << "GetAppIdFromPid";
	return QString();
}

QList<WindowInfo> PlatformApiWindowStack::GetWindowStack() {
	// FIXME Not implemented
	qDebug() << "GetWindowStack";
	return QList<WindowInfo>();
}

QString PlatformApiWindowStack::GetWindowProperty(uint windowId,
		const QString &appId, const QString &name) {
	// FIXME Not implemented
	qDebug() << "GetWindowProperty:" << windowId << appId << name;
	return QString();
}

void PlatformApiWindowStack::onSessionRequested(
		ubuntu_ui_well_known_application app) {
	// FIXME Not implemented
	qDebug() << "onSessionRequested";
}

void PlatformApiWindowStack::onSessionBorn(ubuntu_ui_session_properties props) {
	// FIXME Not implemented
	qDebug() << "onSessionBorn";
	//session_properties_get_window_id
}

void PlatformApiWindowStack::onSessionUnfocused(
		ubuntu_ui_session_properties props) {
	// FIXME Not implemented
	qDebug() << "onSessionUnfocused";
}

void PlatformApiWindowStack::onSessionFocused(
		ubuntu_ui_session_properties props) {
	// FIXME Not implemented
	qDebug() << "onSessionFocused";
}

void PlatformApiWindowStack::onSessionDied(ubuntu_ui_session_properties props) {
	// FIXME Not implemented
	qDebug() << "onSessionDied";
}

void PlatformApiWindowStack::on_session_requested_cb(
		ubuntu_ui_well_known_application app, void* context) {
	reinterpret_cast<PlatformApiWindowStack *>(context)->onSessionRequested(
			app);
}

void PlatformApiWindowStack::on_session_born_cb(
		ubuntu_ui_session_properties props, void* context) {
	reinterpret_cast<PlatformApiWindowStack *>(context)->onSessionBorn(props);
}
void PlatformApiWindowStack::on_session_unfocused_cb(
		ubuntu_ui_session_properties props, void* context) {
	reinterpret_cast<PlatformApiWindowStack *>(context)->onSessionUnfocused(
			props);
}
void PlatformApiWindowStack::on_session_focused_cb(
		ubuntu_ui_session_properties props, void* context) {
	reinterpret_cast<PlatformApiWindowStack *>(context)->onSessionFocused(
			props);
}

void PlatformApiWindowStack::on_session_died_cb(
		ubuntu_ui_session_properties props, void* context) {
	reinterpret_cast<PlatformApiWindowStack *>(context)->onSessionDied(props);
}
