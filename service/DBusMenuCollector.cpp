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
		const QString &applicationId,
		QSharedPointer<ComCanonicalAppMenuRegistrarInterface> appmenu) {
	QDBusPendingReply<QString, QDBusObjectPath> windowReply =
			appmenu->GetMenuForWindow(windowId);

	windowReply.waitForFinished();
	if (windowReply.isError()) {
		return;
	}

	QString service(windowReply.argumentAt<0>());
	QDBusObjectPath path(windowReply.argumentAt<1>());

	if (service.isEmpty()) {
		return;
	}

	qDebug() << "DBusMenu available for" << windowId << "at" << service;

	m_menuImporter.reset(new DBusMenuImporter(service, path.path()));
	m_menu = m_menuImporter->menu();

	QTimer::singleShot(1000, this, SLOT(timeout()));

}

DBusMenuCollector::~DBusMenuCollector() {
}

void DBusMenuCollector::timeout() {
	QList<QAction*> actions = m_menu->actions();
	for (const QAction *action : actions) {
		qDebug() << action->text();
	}
}
