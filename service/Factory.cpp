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

#include <QDBusConnection>

using namespace hud::service;

Factory::Factory() {
}

Factory::~Factory() {
}

HudService::Ptr Factory::singletonHudService() {
	if (m_hudService.isNull()) {
		m_hudService.reset(
				new HudService(*this, QDBusConnection::sessionBus()));
	}
	return m_hudService;
}

Query::Ptr Factory::newQuery(unsigned int id, const QString &query) {
	return Query::Ptr(
			new Query(id, query, *singletonHudService(),
					QDBusConnection::sessionBus()));
}

ApplicationList::Ptr Factory::newApplicationList() {
	return ApplicationList::Ptr(
			new ApplicationList(*this, QDBusConnection::sessionBus()));
}

Application::Ptr Factory::newApplication(unsigned int id,
		const QString &applicationId) {
	return Application::Ptr(
			new Application(id, applicationId, QDBusConnection::sessionBus()));
}
