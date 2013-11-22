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

#include <service/Factory.h>
#include <service/WindowImpl.h>
#include <QDebug>

using namespace hud::service;

WindowTokenImpl::WindowTokenImpl(Collector::Ptr dbusMenuCollector,
		Collector::Ptr gMenuCollector) {
	if (dbusMenuCollector->isValid()) {
		m_dbusMenuToken = dbusMenuCollector->activate();
		m_items.indexMenu(dbusMenuCollector->menu());
	}
	if (gMenuCollector->isValid()) {
		m_gMenuToken = gMenuCollector->activate();
		m_items.indexMenu(gMenuCollector->menu());
	}
}

WindowTokenImpl::~WindowTokenImpl() {
}

void WindowTokenImpl::search(const QString &query, QList<Result> &results) {
	m_items.search(query, results);
}

void WindowTokenImpl::execute(unsigned long long commandId, uint timestamp) {
	m_items.execute(commandId, timestamp);
}

void WindowTokenImpl::commands(QList<QStringList> &commandsList) {
	m_items.commands(commandsList);
}

WindowImpl::WindowImpl(unsigned int windowId, const QString &applicationId,
		WindowContext::Ptr allWindowsContext, Factory &factory) :
		WindowContextImpl(factory), m_allWindowsContext(allWindowsContext) {

	m_dbusMenuCollector = factory.newDBusMenuCollector(windowId, applicationId);
	m_gMenuCollector = factory.newGMenuCollector(windowId, applicationId);
}

WindowImpl::~WindowImpl() {
}

WindowToken::Ptr WindowImpl::activate() {
	WindowToken::Ptr windowToken(m_windowToken);

	if (windowToken.isNull()) {
		windowToken.reset(
				new WindowTokenImpl(m_dbusMenuCollector, m_gMenuCollector));

		//FIXME Temporary debug code
		std::shared_ptr<QMenu> menu = m_allWindowsContext->activeMenu();
		if (menu) {
			qDebug() << "we have a menu";
			for (QAction *action : menu->actions()) {
				qDebug() << action->text();
			}
		} else {
			qDebug() << "we have no menu";
		}

		m_windowToken = windowToken;
	}

	return windowToken;
}
