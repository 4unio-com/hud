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

#ifndef PLATFORMAPIWINDOWSTACK_H_
#define PLATFORMAPIWINDOWSTACK_H_

#include <AbstractWindowStack.h>

#include <ubuntu/ui/ubuntu_ui_session_service.h>

class Q_DECL_EXPORT PlatformApiWindowStack: public AbstractWindowStack {
Q_OBJECT

public:
	explicit PlatformApiWindowStack(const QDBusConnection &connection,
			QObject *parent = 0);

	virtual ~PlatformApiWindowStack();

public Q_SLOTS:
	virtual QString GetAppIdFromPid(uint pid);

	virtual QList<WindowInfo> GetWindowStack();

	virtual QStringList GetWindowProperties(uint windowId, const QString &appId,
			const QStringList &names);

protected:
	static void on_session_requested_cb(ubuntu_ui_well_known_application app,
			void* context);

	void onSessionRequested(ubuntu_ui_well_known_application app);

	static void on_session_born_cb(ubuntu_ui_session_properties props,
			void* context);

	void onSessionBorn(ubuntu_ui_session_properties props);

	static void on_session_unfocused_cb(ubuntu_ui_session_properties props,
			void* context);

	void onSessionUnfocused(ubuntu_ui_session_properties props);

	static void on_session_focused_cb(ubuntu_ui_session_properties props,
			void* context);

	void onSessionFocused(ubuntu_ui_session_properties props);

	static void on_session_died_cb(ubuntu_ui_session_properties props,
			void* context);

	void onSessionDied(ubuntu_ui_session_properties props);

	ubuntu_ui_session_lifecycle_observer m_observer_definition;
};

#endif /* PLATFORMAPIWINDOWSTACK_H_ */
