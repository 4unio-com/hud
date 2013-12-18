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
#include <service/HudServiceImpl.h>
#include <service/WindowImpl.h>
#include <service/QueryImpl.h>
#include <service/VoiceImpl.h>
#include <service/WindowImpl.h>
#include <common/DBusTypes.h>

#include <QDBusConnection>
#include <QDBusServiceWatcher>

using namespace hud::common;
using namespace hud::service;

Factory::Factory() :
		m_sessionBus(QDBusConnection::sessionBus()), m_applicationCounter(0), m_queryCounter(
				0) {
	DBusTypes::registerMetaTypes();
	LibUnityVoice::UnityVoice::registerMetaTypes();
}

Factory::~Factory() {
}

void Factory::setSessionBus(const QDBusConnection &sessionBus) {
	m_sessionBus = sessionBus;
}

HudService::Ptr Factory::singletonHudService() {
	if (m_hudService.isNull()) {
		m_hudService.reset(
				new HudServiceImpl(*this, singletonApplicationList(),
						sessionBus()));
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

QSharedPointer<QDBusServiceWatcher> Factory::windowStackWatcher() {
	return QSharedPointer<QDBusServiceWatcher>(
			new QDBusServiceWatcher(DBusTypes::WINDOW_STACK_DBUS_NAME,
					sessionBus(), QDBusServiceWatcher::WatchForUnregistration));
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

Query::Ptr Factory::newQuery(const QString &query, const QString &sender) {
	return Query::Ptr(
			new QueryImpl(m_queryCounter++, query, sender,
					*singletonHudService(), singletonApplicationList(),
					singletonVoice(), sessionBus()));
}

ApplicationList::Ptr Factory::singletonApplicationList() {
	if (m_applicationList.isNull()) {
		m_applicationList.reset(
				new ApplicationListImpl(*this, singletonWindowStack(),
						windowStackWatcher()));
	}
	return m_applicationList;
}

Voice::Ptr Factory::singletonVoice() {
	if (m_voice.isNull()) {
		QSharedPointer<ComCanonicalUnityVoiceInterface> voiceInterface(
				new ComCanonicalUnityVoiceInterface(
						DBusTypes::UNITY_VOICE_DBUS_NAME,
						DBusTypes::UNITY_VOICE_DBUS_PATH, sessionBus()));
		m_voice.reset(new VoiceImpl(voiceInterface));
	}
	return m_voice;
}

Application::Ptr Factory::newApplication(const QString &applicationId) {
	return Application::Ptr(
			new ApplicationImpl(m_applicationCounter++, applicationId, *this,
					sessionBus()));
}

ItemStore::Ptr Factory::newItemStore() {
	return ItemStore::Ptr(new ItemStore());
}

Window::Ptr Factory::newWindow(unsigned int windowId,
		const QString &applicationId, WindowContext::Ptr allwindowsContext) {
	return Window::Ptr(
			new WindowImpl(windowId, applicationId, allwindowsContext, *this));
}

WindowContext::Ptr Factory::newWindowContext() {
	return WindowContext::Ptr(new WindowContextImpl(*this));
}

Collector::Ptr Factory::newDBusMenuCollector(unsigned int windowId,
		const QString &applicationId) {
	Q_UNUSED(applicationId);
	return Collector::Ptr(new DBusMenuCollector(windowId, singletonAppmenu()));
}

Collector::Ptr Factory::newGMenuCollector(const QString &name,
		const QDBusObjectPath &actionPath, const QDBusObjectPath &menuPath) {
	return Collector::Ptr(new GMenuCollector(name, actionPath, menuPath));
}

Collector::Ptr Factory::newGMenuWindowCollector(unsigned int windowId,
		const QString &applicationId) {
	return Collector::Ptr(
			new GMenuWindowCollector(windowId, applicationId,
					singletonWindowStack(), *this));
}
