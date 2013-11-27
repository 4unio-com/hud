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

#ifndef HUD_SERVICE_APPLICATIONLISTIMPL_H_
#define HUD_SERVICE_APPLICATIONLISTIMPL_H_

#include <common/WindowStackInterface.h>
#include <service/ApplicationList.h>
#include <service/Application.h>

#include <QObject>
#include <QDBusConnection>
#include <QSharedPointer>

namespace hud {
namespace service {

class Factory;

class ApplicationListImpl: public ApplicationList {
Q_OBJECT

public:
	explicit ApplicationListImpl(Factory &factory,
			QSharedPointer<ComCanonicalUnityWindowStackInterface> windowStack);

	virtual ~ApplicationListImpl();

	virtual QList<hud::common::NameObject> applications() const;

	virtual Application::Ptr focusedApplication() const;

	virtual Window::Ptr focusedWindow() const;

	virtual Application::Ptr ensureApplication(const QString &applicationId);

public Q_SLOTS:
	void FocusedWindowChanged(uint windowId, const QString &applicationId,
			uint stage);

	void WindowCreated(uint windowId, const QString &applicationId);

	void WindowDestroyed(uint windowId, const QString &applicationId);

protected:
	void ensureApplicationWithWindow(uint windowId,
			const QString& applicationId);

	void removeWindow(uint windowId, const QString& applicationId);

	void setFocusedWindow(Application::Ptr application, uint windowId);

	static bool isIgnoredApplication(const QString &applicationId);

	QSharedPointer<ComCanonicalUnityWindowStackInterface> m_windowStack;

	Factory &m_factory;

	QMap<QString, Application::Ptr> m_applications;

	Application::Ptr m_focusedApplication;

	uint m_focusedWindowId;
};

}
}

#endif /* HUD_SERVICE_APPLICATIONLIST_IMPLH_ */
