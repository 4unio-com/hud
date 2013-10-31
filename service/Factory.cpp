/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY{} without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Pete Woods <pete.woods@canonical.com>
 */

#include <service/Factory.h>
#include <service/ApplicationImpl.h>
#include <service/ApplicationListImpl.h>
#include <service/AppmenuRegistrarInterface.h>
#include <service/QueryImpl.h>
#include <service/WindowImpl.h>
#include <common/DBusTypes.h>

#include <QDBusConnection>

using namespace hud::common;
using namespace hud::service;

Factory::Factory() :
		m_sessionBus(QDBusConnection::sessionBus()) {
}

Factory::~Factory() {
}

void Factory::setSessionBus(const QDBusConnection &sessionBus) {
	m_sessionBus = sessionBus;
}

HudService::Ptr Factory::singletonHudService() {
	if (m_hudService.isNull()) {
		m_hudService.reset(new HudService(*this, sessionBus()));
	}
	return m_hudService;
}

QSharedPointer<ComCanonicalUnityWindowStackInterface> Factory::singletonWindowStack() {
	if (m_windowStack.isNull()) {
		m_windowStack.reset(
				new ComCanonicalUnityWindowStackInterface(
						DBusTypes::WINDOW_STACK_DBUS_NAME,
						DBusTypes::WINDOW_STACK_DBUS_PATH, sessionBus()));
	}
	return m_windowStack;

}

QSharedPointer<ComCanonicalAppMenuRegistrarInterface> Factory::singletonAppmenu() {
	if (m_appmenu.isNull()) {
		m_appmenu.reset(
				new ComCanonicalAppMenuRegistrarInterface(
						DBusTypes::APPMENU_REGISTRAR_DBUS_NAME,
						DBusTypes::APPMENU_REGISTRAR_DBUS_PATH, sessionBus()));
	}
	return m_appmenu;
}

QDBusConnection Factory::sessionBus() {
	return m_sessionBus;
}

Query::Ptr Factory::newQuery(unsigned int id, const QString &query) {
	return Query::Ptr(
			new QueryImpl(id, query, *singletonHudService(), sessionBus()));
}

ApplicationList::Ptr Factory::newApplicationList() {
	return ApplicationList::Ptr(
			new ApplicationListImpl(*this, singletonWindowStack()));
}

Application::Ptr Factory::newApplication(unsigned int id,
		const QString &applicationId) {
	return Application::Ptr(
			new ApplicationImpl(id, applicationId, *this, sessionBus()));
}

ItemStore::Ptr Factory::newItemStore() {
	return ItemStore::Ptr(new ItemStore());
}

Window::Ptr Factory::newWindow(unsigned int windowId,
		const QString &applicationId) {
	return Window::Ptr(new WindowImpl(windowId, applicationId, *this));
}

DBusMenuCollector::Ptr Factory::newDBusMenuCollector(unsigned int windowId,
		const QString &applicationId) {
	return DBusMenuCollector::Ptr(
			new DBusMenuCollector(windowId, applicationId, singletonAppmenu()));
}

GMenuCollector::Ptr Factory::newGMenuCollector(unsigned int windowId,
		const QString &applicationId) {
	return GMenuCollector::Ptr(
			new GMenuCollector(windowId, applicationId, singletonWindowStack()));
}
