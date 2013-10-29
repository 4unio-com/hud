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

#ifndef HUD_SERVICE_DBUSMENUCOLLECTOR_H_
#define HUD_SERVICE_DBUSMENUCOLLECTOR_H_

#include <QObject>
#include <QSharedPointer>
#include <QDBusConnection>

class ComCanonicalAppMenuRegistrarInterface;
class DBusMenuImporter;

QT_BEGIN_NAMESPACE
class QMenu;
QT_END_NAMESPACE

namespace hud {
namespace service {

class DBusMenuCollector: public QObject {
Q_OBJECT
public:
	typedef QSharedPointer<DBusMenuCollector> Ptr;

	explicit DBusMenuCollector(unsigned int windowId,
			const QString &applicationId,
			QSharedPointer<ComCanonicalAppMenuRegistrarInterface> appmenu);

	virtual ~DBusMenuCollector();

protected Q_SLOTS:
	void timeout();

protected:

	QScopedPointer<DBusMenuImporter> m_menuImporter;

	QMenu *m_menu;
};

}
}

#endif /* HUD_SERVICE_DBUSMENUCOLLECTOR_H_ */
