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

	m_activeAction = m_actions[context];
	m_activeMenu = m_menus[context];
}

void WindowContextImpl::addAction(const QString &context, const QString &name,
		const QDBusObjectPath &path, const QString &prefix) {
	qDebug() << "WindowContextImpl::addAction" << context << name << path.path()
			<< prefix;

	QSharedPointer<QtGMenuImporter> importer(
			new qtgmenu::QtGMenuImporter(name, QString(), path.path()));

	m_actions[context] = importer;
}

void WindowContextImpl::addModel(const QString &context, const QString &name,
		const QDBusObjectPath &path) {
	qDebug() << "WindowContextImpl::addModel" << context << name << path.path();

	QSharedPointer<QtGMenuImporter> importer(
			new qtgmenu::QtGMenuImporter(name, path.path(), QString()));

	m_menus[context] = importer;
}

std::shared_ptr<QMenu> WindowContextImpl::activeAction() const {
	std::shared_ptr<QMenu> action;
	if (m_activeAction) {
		qDebug() << "We have an active action collector";
		action = m_activeAction->GetQMenu();
	}
	return action;
}

std::shared_ptr<QMenu> WindowContextImpl::activeMenu() const {
	std::shared_ptr<QMenu> menu;
	if (m_activeMenu) {
		qDebug() << "We have an active menu collector";
		menu = m_activeMenu->GetQMenu();
	}
	return menu;
}
