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

#include <common/WindowStackInterface.h>
#include <service/GMenuCollector.h>

#include <QStringList>
#include <QDebug>

using namespace hud::service;

static const QStringList GMENU_WINDOW_PROPERTIES( { "_GTK_UNIQUE_BUS_NAME",
		"_GTK_APP_MENU_OBJECT_PATH", "_GTK_MENUBAR_OBJECT_PATH",
		"_GTK_APPLICATION_OBJECT_PATH", "_GTK_WINDOW_OBJECT_PATH",
		"_UNITY_OBJECT_PATH" });

GMenuCollector::GMenuCollector(unsigned int windowId,
		const QString &applicationId,
		QSharedPointer<ComCanonicalUnityWindowStackInterface> windowStack) {
	QDBusPendingReply<QStringList> windowPropertiesReply(
			windowStack->GetWindowProperties(windowId, applicationId,
					GMENU_WINDOW_PROPERTIES));

	windowPropertiesReply.waitForFinished();
	if (windowPropertiesReply.isError()) {
		qWarning() << windowPropertiesReply.error();
		return;
	}
	QStringList windowProperties(windowPropertiesReply);

	// If we have the bus name property, then lets look for a GMenu
	if (!windowProperties.at(0).isEmpty()) {
		qDebug() << "GMenu available for" << windowId << "at"
				<< windowProperties.at(0);
	}
}

GMenuCollector::~GMenuCollector() {
}
