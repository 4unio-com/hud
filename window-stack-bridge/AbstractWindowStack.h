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

#ifndef ABSTRACTWINDOWSTACK_H_
#define ABSTRACTWINDOWSTACK_H_

#include <QList>
#include <QString>
#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusContext>
#include <QScopedPointer>

class WindowStackAdaptor;

class WindowInfo {
public:
	enum Stage {
		MAIN, SIDE, WINDOWED,
	};

	unsigned int window_id;
	QString app_id;
	bool focused;
	unsigned int stage;

	WindowInfo() :
			window_id(0), focused(false), stage(MAIN) {
	}

	WindowInfo(unsigned int window_id, const QString &app_id, bool focused,
			Stage stage = MAIN) :
			window_id(window_id), app_id(app_id), focused(focused), stage(stage) {
	}
};

Q_DECL_EXPORT
QDBusArgument &operator<<(QDBusArgument &a, const WindowInfo &aidf);

Q_DECL_EXPORT
const QDBusArgument &operator>>(const QDBusArgument &a, WindowInfo &aidf);

class Q_DECL_EXPORT AbstractWindowStack: public QObject, protected QDBusContext {
Q_OBJECT

public:
	explicit AbstractWindowStack(const QDBusConnection &connection,
			QObject *parent = 0);

	virtual ~AbstractWindowStack();

public:
	Q_DECL_EXPORT
	static const QString DBUS_NAME;

	Q_DECL_EXPORT
	static const QString DBUS_PATH;

	Q_DECL_EXPORT
	static void registerMetaTypes();

public Q_SLOTS:
	virtual QString GetAppIdFromPid(uint pid) = 0;

	virtual QList<WindowInfo> GetWindowStack() = 0;

	virtual QString GetWindowProperty(uint windowId, const QString &appId,
			const QString &name) = 0;

Q_SIGNALS:
	void FocusedWindowChanged(uint windowId, const QString &appId, uint stage);

	void WindowCreated(uint windowId, const QString &appId);

	void WindowDestroyed(uint windowId, const QString &appId);

protected:
	QScopedPointer<WindowStackAdaptor> m_adaptor;

	QDBusConnection m_connection;
};

Q_DECLARE_METATYPE(WindowInfo);

#endif /* ABSTRACTWINDOWSTACK_H_ */
