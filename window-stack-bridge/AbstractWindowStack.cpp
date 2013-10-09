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

#include <AbstractWindowStack.h>
#include <WindowStackAdaptor.h>
#include <Localisation.h>

#include <stdexcept>
#include <QDBusMetaType>
#include <QDebug>

const QString AbstractWindowStack::DBUS_NAME("com.canonical.Unity.WindowStack");
const QString AbstractWindowStack::DBUS_PATH(
		"/com/canonical/Unity/WindowStack");

QDBusArgument & operator<<(QDBusArgument &a, const WindowInfo &wi) {
	a.beginStructure();
	a << wi.window_id << wi.app_id << wi.focused << wi.stage;
	a.endStructure();
	return a;
}

const QDBusArgument & operator>>(const QDBusArgument &a, WindowInfo &wi) {
	a.beginStructure();
	uint stage;
	a >> wi.window_id >> wi.app_id >> wi.focused >> stage;
	a.endStructure();
	wi.stage = static_cast<WindowInfo::Stage>(stage);
	return a;
}

AbstractWindowStack::AbstractWindowStack(const QDBusConnection &connection,
		QObject *parent) :
		QObject(parent), m_adaptor(new WindowStackAdaptor(this)), m_connection(
				connection) {
	registerMetaTypes();
}

void AbstractWindowStack::registerOnBus() {
	if (!m_connection.registerObject(DBUS_PATH, this)) {
		throw std::logic_error(
				_("Unable to register window stack object on DBus"));
	}
	if (!m_connection.registerService(DBUS_NAME)) {
		throw std::logic_error(
				_("Unable to register window stack service on DBus"));
	}
}

AbstractWindowStack::~AbstractWindowStack() {
	m_connection.unregisterObject(DBUS_PATH);
}

void AbstractWindowStack::registerMetaTypes() {
	qRegisterMetaType<WindowInfo>();
	qRegisterMetaType<QList<WindowInfo>>();
	qDBusRegisterMetaType<WindowInfo>();
	qDBusRegisterMetaType<QList<WindowInfo>>();
}

