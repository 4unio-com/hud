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

#include <common/DBusTypes.h>
#include <service/DBusMenuCollector.h>
#include <service/AppmenuRegistrarInterface.h>

#include <dbusmenuimporter.h>
#include <QMenu>

using namespace hud::common;
using namespace hud::service;

DBusMenuCollector::DBusMenuCollector(unsigned int windowId,
		QSharedPointer<ComCanonicalAppMenuRegistrarInterface> appmenu) :
		m_valid(false), m_menu(nullptr) {
	QDBusPendingReply<QString, QDBusObjectPath> windowReply =
			appmenu->GetMenuForWindow(windowId);

	windowReply.waitForFinished();
	if (windowReply.isError()) {
		return;
	}

	m_service = windowReply.argumentAt<0>();
	m_path = windowReply.argumentAt<1>();

	if (m_service.isEmpty()) {
		return;
	}

	m_valid = true;

	qDebug() << "DBusMenu available for" << windowId << "at" << m_service
			<< m_path.path();
}

DBusMenuCollector::~DBusMenuCollector() {
}

bool DBusMenuCollector::isValid() const {
	return m_valid;
}

void DBusMenuCollector::collect() {
	m_menuImporter.reset(new DBusMenuImporter(m_service, m_path.path()));
	m_menu = m_menuImporter->menu();

	QTimer::singleShot(50, this, SLOT(timeout()));
}

void DBusMenuCollector::timeout() {
	QList<QAction*> actions = m_menu->actions();
	for (const QAction *action : actions) {
		qDebug() << action->text();
	}
}
