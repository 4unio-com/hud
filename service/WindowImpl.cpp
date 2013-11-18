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
		gMenuCollector->activate();
		m_items.indexMenu(gMenuCollector->menu());
	}
}

WindowTokenImpl::~WindowTokenImpl() {
}

void WindowTokenImpl::search(const QString &query, QList<Result> &results) {
	m_items.search(query, results);
}

WindowImpl::WindowImpl(unsigned int windowId, const QString &applicationId,
		Factory &factory) :
		m_dbusMenuCollector(
				factory.newDBusMenuCollector(windowId, applicationId)), m_gMenuCollector(
				factory.newGMenuCollector(windowId, applicationId)) {
}

WindowImpl::~WindowImpl() {
}

WindowToken::Ptr WindowImpl::activate() {
	WindowToken::Ptr windowToken(m_windowToken);

	if (windowToken.isNull()) {
		windowToken.reset(
				new WindowTokenImpl(m_dbusMenuCollector, m_gMenuCollector));
		m_windowToken = windowToken;
	}

	return windowToken;
}

void WindowImpl::setContext(const QString &context) {
	m_context = context;
	qDebug() << "Window updated context to" << context;
}

