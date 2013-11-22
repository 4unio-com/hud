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
#include <service/WindowContextImpl.h>
#include <QDebug>

#include <libqtgmenu/QtGMenuImporter.h>

using namespace hud::service;
using namespace qtgmenu;

WindowContextImpl::WindowContextImpl(Factory &factory) {
}

WindowContextImpl::~WindowContextImpl() {
}

void WindowContextImpl::setContext(const QString &context) {
	m_context = context;
	qDebug() << "WindowContextImpl::setContext" << context;

	m_activeMenu = m_menus[context];
}

void WindowContextImpl::addMenu(const QString &context,
		const MenuDefinition &menuDefinition) {
	qDebug() << "WindowContextImpl::addModel" << context << menuDefinition.name
			<< menuDefinition.actionPath.path() << menuDefinition.actionPrefix
			<< menuDefinition.menuPath.path();

	QSharedPointer<QtGMenuImporter> importer(
			new qtgmenu::QtGMenuImporter(menuDefinition.name,
					menuDefinition.menuPath.path(),
					menuDefinition.actionPath.path()));

	m_menus[context] = importer;
}

std::shared_ptr<QMenu> WindowContextImpl::activeMenu() const {
	std::shared_ptr<QMenu> menu;
	if (m_activeMenu) {
		qDebug() << "We have an active menu collector";
		menu = m_activeMenu->GetQMenu();
	}
	return menu;
}
