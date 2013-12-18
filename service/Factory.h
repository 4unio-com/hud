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

#ifndef HUD_SERVICE_FACTORY_H_
#define HUD_SERVICE_FACTORY_H_

#include <service/HudService.h>
#include <service/Application.h>
#include <service/ApplicationList.h>
#include <service/DBusMenuCollector.h>
#include <service/GMenuWindowCollector.h>
#include <service/GMenuCollector.h>
#include <service/ItemStore.h>
#include <service/Voice.h>
#include <service/Query.h>
#include <service/Window.h>

class ComCanonicalUnityWindowStackInterface;
class ComCanonicalAppMenuRegistrarInterface;

QT_BEGIN_NAMESPACE
class QDBusServiceWatcher;
QT_END_NAMESPACE

namespace hud {
namespace service {

class Factory {
public:
	explicit Factory();

	virtual ~Factory();

	void setSessionBus(const QDBusConnection &sessionBus);

	virtual HudService::Ptr singletonHudService();

	virtual QDBusConnection sessionBus();

	virtual QSharedPointer<ComCanonicalUnityWindowStackInterface> singletonWindowStack();

	virtual QSharedPointer<QDBusServiceWatcher> windowStackWatcher();

	virtual QSharedPointer<ComCanonicalAppMenuRegistrarInterface> singletonAppmenu();

	virtual Query::Ptr newQuery(const QString &query, const QString &sender);

	virtual ApplicationList::Ptr singletonApplicationList();

	virtual Voice::Ptr singletonVoice();

	virtual Application::Ptr newApplication(const QString &applicationId);

	virtual ItemStore::Ptr newItemStore();

	virtual Window::Ptr newWindow(unsigned int windowId,
			const QString &applicationId, WindowContext::Ptr allwindowsContext);

	virtual WindowContext::Ptr newWindowContext();

	virtual Collector::Ptr newDBusMenuCollector(unsigned int windowId,
			const QString &applicationId);

	virtual Collector::Ptr newGMenuCollector(const QString &name,
			const QDBusObjectPath &actionPath, const QDBusObjectPath &menuPath);

	virtual Collector::Ptr newGMenuWindowCollector(unsigned int windowId,
			const QString &applicationId);

protected:
	QDBusConnection m_sessionBus;

	unsigned int m_queryCounter;

	HudService::Ptr m_hudService;

	ApplicationList::Ptr m_applicationList;

	Voice::Ptr m_voice;

	QSharedPointer<ComCanonicalUnityWindowStackInterface> m_windowStack;

	QSharedPointer<ComCanonicalAppMenuRegistrarInterface> m_appmenu;
};

}
}

#endif /* HUD_SERVICE_FACTORY_H_ */
