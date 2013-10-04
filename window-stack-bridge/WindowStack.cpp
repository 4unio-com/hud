/*
 * WindowStack.cpp
 *
 *  Created on: 4 Oct 2013
 *      Author: pete
 */

#include <WindowStack.h>
#include <WindowStackAdaptor.h>
#include <Localisation.h>

#include <stdexcept>
#include <QDBusMetaType>
#include <QDebug>

const QString WindowStack::DBUS_NAME("com.canonical.Unity.WindowStack");
const QString WindowStack::DBUS_PATH("/com/canonical/Unity/WindowStack");

QDBusArgument & operator<<(QDBusArgument &a, const WindowInfo &wi) {
	a.beginStructure();
	a << wi.window_id << wi.app_id << wi.focused << wi.stage;
	a.endStructure();
	return a;
}

const QDBusArgument & operator>>(const QDBusArgument &a, WindowInfo &wi) {
	a.beginStructure();
	a >> wi.window_id >> wi.app_id >> wi.focused >> wi.stage;
	a.endStructure();
	return a;
}

WindowStack::WindowStack(const QDBusConnection &connection, QObject *parent) :
		QObject(parent), m_adaptor(new WindowStackAdaptor(this)), m_connection(
				connection) {
	registerMetaTypes();
	if (!m_connection.registerService(DBUS_NAME)) {
		throw std::logic_error(
				_("Unable to register window stack service on DBus"));
	}
	if (!m_connection.registerObject(DBUS_PATH, this)) {
		throw std::logic_error(
				_("Unable to register window stack object on DBus"));
	}
}

WindowStack::~WindowStack() {
	m_connection.unregisterObject(DBUS_PATH);
}

void WindowStack::registerMetaTypes() {
	qRegisterMetaType<WindowInfo>("WindowInfo");
	qDBusRegisterMetaType<WindowInfo>();
}

QString WindowStack::GetAppIdFromPid(uint pid) {
	return QString();
}

QList<WindowInfo> WindowStack::GetWindowStack() {
	return QList<WindowInfo>();
}

