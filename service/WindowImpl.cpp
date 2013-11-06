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

WindowImpl::WindowImpl(unsigned int windowId, const QString &applicationId,
		Factory &factory) :
		m_dbusMenuCollector(
				factory.newDBusMenuCollector(windowId, applicationId)), m_gMenuCollector(
				factory.newGMenuCollector(windowId, applicationId)) {
}

WindowImpl::~WindowImpl() {
}

void WindowImpl::activate() {
	if (m_dbusMenuCollector->isValid()) {
		m_dbusMenuCollector->activate();
	}
	if (m_gMenuCollector->isValid()) {
		m_gMenuCollector->activate();
	}
}
