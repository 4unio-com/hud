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
		QSharedPointer<ComCanonicalUnityWindowStackInterface> windowStack) :
		m_windowStack(windowStack), m_valid(false) {

	QDBusPendingReply<QStringList> windowPropertiesReply(
			windowStack->GetWindowProperties(windowId, applicationId,
					GMENU_WINDOW_PROPERTIES));

	windowPropertiesReply.waitForFinished();
	if (windowPropertiesReply.isError()) {
		qWarning() << windowPropertiesReply.error();
		return;
	}
	QStringList windowProperties(windowPropertiesReply);

	if (windowProperties.isEmpty()) {
		return;
	}

	m_busName = windowProperties.at(0);

	// We're using the existence of the bus name property to determine
	// if this window has GMenus available at all.
	if (m_busName.isEmpty()) {
		return;
	}

	if (!windowProperties.at(1).isEmpty()) {
		m_appmenuPath = QDBusObjectPath(windowProperties.at(1));
	}
	if (!windowProperties.at(2).isEmpty()) {
		m_menubarPath = QDBusObjectPath(windowProperties.at(2));
	}
	if (!windowProperties.at(3).isEmpty()) {
		m_applicationPath = QDBusObjectPath(windowProperties.at(3));
	}
	if (!windowProperties.at(4).isEmpty()) {
		m_windowPath = QDBusObjectPath(windowProperties.at(4));
	}
	if (!windowProperties.at(5).isEmpty()) {
		m_unityPath = QDBusObjectPath(windowProperties.at(5));
	}

	m_valid = true;
	qDebug() << "GMenu available for" << applicationId << windowId << "at"
			<< m_busName;
	qDebug() << "m_appmenuPath" << m_appmenuPath.path();
	qDebug() << "m_menubarPath" << m_menubarPath.path();
	qDebug() << "m_applicationPath" << m_applicationPath.path();
	qDebug() << "m_windowPath" << m_windowPath.path();
	qDebug() << "m_unityPath" << m_unityPath.path();
}

GMenuCollector::~GMenuCollector() {
}

bool GMenuCollector::isValid() const {
	return m_valid;
}

CollectorToken::Ptr GMenuCollector::activate() {
	return CollectorToken::Ptr(new CollectorToken(*this));
}

void GMenuCollector::deactivate() {
}
