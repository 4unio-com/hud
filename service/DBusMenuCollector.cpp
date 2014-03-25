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
#include <common/Localisation.h>
#include <service/DBusMenuCollector.h>
#include <service/AppmenuRegistrarInterface.h>

#include <dbusmenuimporter.h>
#include <QMenu>

using namespace hud::common;
using namespace hud::service;

DBusMenuCollector::DBusMenuCollector(unsigned int windowId,
		QSharedPointer<ComCanonicalAppMenuRegistrarInterface> registrar) :
		m_windowId(windowId), m_registrar(registrar) {

	connect(registrar.data(),
	SIGNAL(WindowRegistered(uint, const QString &, const QDBusObjectPath &)),
			this,
			SLOT( WindowRegistered(uint, const QString &, const QDBusObjectPath &)));

	QDBusPendingReply<QString, QDBusObjectPath> windowReply =
			registrar->GetMenuForWindow(m_windowId);

	windowReply.waitForFinished();
	if (windowReply.isError()) {
		return;
	}

	windowRegistered(windowReply.argumentAt<0>(), windowReply.argumentAt<1>());
}

DBusMenuCollector::~DBusMenuCollector() {
}

void DBusMenuCollector::windowRegistered(const QString &service,
		const QDBusObjectPath &menuObjectPath) {

	if (service.isEmpty()) {
		return;
	}

	m_service = service;
	m_path = menuObjectPath;

	disconnect(m_registrar.data(),
			SIGNAL(
					WindowRegistered(uint, const QString &, const QDBusObjectPath &)),
			this,
			SLOT(
					WindowRegistered(uint, const QString &, const QDBusObjectPath &)));

	m_menuImporter.reset(
			new DBusMenuImporter(m_service, m_path.path(),
					DBusMenuImporterType::SYNCHRONOUS));

	CollectorToken::Ptr collectorToken(m_collectorToken);
	if(collectorToken) {
		collectorToken->changed();
	}
}

bool DBusMenuCollector::isValid() const {
	return !m_menuImporter.isNull();
}

inline uint qHash(const QStringList &key, uint seed) {
	uint hash(0);
	for (const QString &s : key) {
		hash ^= qHash(s, seed);
	}
	return hash;
}

void DBusMenuCollector::openMenu(QMenu *menu, unsigned int &limit) {
	if (!menu) {
		return;
	}

	if (limit == 0) {
		qWarning() << "Hit DBusMenu safety valve for menu at" << m_service
				<< m_path.path();
		return;
	}

	menu->aboutToShow();

	for (int i(0); m_menuImporter && i < menu->actions().size(); ++i) {

		QAction *action = menu->actions().at(i);
		if (!action->isEnabled()) {
			continue;
		}
		if (action->isSeparator()) {
			continue;
		}

		QMenu *child(action->menu());
		if (child) {
			--limit;
			openMenu(child, limit);
		}
	}
}

void DBusMenuCollector::hideMenu(QMenu *menu) {
	for (int i(0); i < menu->actions().size(); ++i) {
		QAction *action = menu->actions().at(i);
		QMenu *child(action->menu());
		if (child) {
			hideMenu(child);
		}
	}

	menu->aboutToHide();

	if(!m_menuImporter) {
		return;
	}
}

QList<CollectorToken::Ptr> DBusMenuCollector::activate() {
	CollectorToken::Ptr collectorToken(m_collectorToken);

	if(m_menuImporter.isNull()) {
		return QList<CollectorToken::Ptr>();
	}

	if (collectorToken.isNull()) {
		unsigned int limit(50);
		openMenu(m_menuImporter->menu(), limit);

		if(m_menuImporter.isNull()) {
			return QList<CollectorToken::Ptr>();
		}

		collectorToken.reset(
				new CollectorToken(shared_from_this(), m_menuImporter->menu()));
		m_collectorToken = collectorToken;
	}

	return QList<CollectorToken::Ptr>() << collectorToken;
}

void DBusMenuCollector::deactivate() {
	if(m_menuImporter.isNull()) {
		return;
	}
	hideMenu(m_menuImporter->menu());
}

void DBusMenuCollector::WindowRegistered(uint windowId, const QString &service,
		const QDBusObjectPath &menuObjectPath) {
	// Simply ignore updates for other windows
	if (windowId != m_windowId) {
		return;
	}

	windowRegistered(service, menuObjectPath);
}
