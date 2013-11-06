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

#ifndef HUD_SERVICE_APPLICATIONIMPL_H_
#define HUD_SERVICE_APPLICATIONIMPL_H_

#include <common/Action.h>
#include <common/ActionGroup.h>
#include <common/Description.h>
#include <common/MenuModel.h>
#include <service/Application.h>
#include <service/Window.h>

#include <QDBusConnection>
#include <QDBusContext>
#include <QMap>
#include <QScopedPointer>
#include <QSharedPointer>

class ApplicationAdaptor;

namespace hud {
namespace service {

class Factory;

class Q_DECL_EXPORT ApplicationImpl: public Application, protected QDBusContext {
Q_OBJECT

public:
	explicit ApplicationImpl(unsigned int id, const QString &applicationId,
			Factory &factory, const QDBusConnection &connection,
			QObject *parent = 0);

	virtual ~ApplicationImpl();

	void addWindow(unsigned int windowId);

	void removeWindow(unsigned int windowId);

	void activateWindow(unsigned int windowId);

	bool isEmpty() const;

	const QDBusObjectPath & path() const;

	Q_PROPERTY(QList<hud::common::ActionGroup> ActionGroups READ actionGroups)
	QList<hud::common::ActionGroup> actionGroups() const;

	Q_PROPERTY(QString DesktopPath READ desktopPath)
	QString desktopPath();

	Q_PROPERTY(QString Icon READ icon)
	QString icon();

	Q_PROPERTY(QList<hud::common::MenuModel> MenuModels READ menuModels)
	QList<hud::common::MenuModel> menuModels() const;

public Q_SLOTS:
	void AddSources(const QList<hud::common::Action> &actions,
			const QList<hud::common::Description> &descriptions);

	void SetWindowContext(uint window, const QString &context);

protected:
	QScopedPointer<ApplicationAdaptor> m_adaptor;

	QDBusConnection m_connection;

	QDBusObjectPath m_path;

	QString m_applicationId;

	Factory &m_factory;

	QMap<unsigned int, Window::Ptr> m_windows;

	QString m_desktopPath;

	QString m_icon;
};

}
}

#endif /* HUD_SERVICE_APPLICATIONIMPL_H_ */
