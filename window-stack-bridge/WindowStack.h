/*
 * WindowStack.h
 *
 *  Created on: 4 Oct 2013
 *      Author: pete
 */

#ifndef WINDOWSTACK_H_
#define WINDOWSTACK_H_

#include <QList>
#include <QString>
#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusContext>
#include <QScopedPointer>

class WindowStackAdaptor;

class WindowInfo {
public:
	unsigned int window_id;
	QString app_id;
	bool focused;
	unsigned int stage;
};

QDBusArgument &operator<<(QDBusArgument &a, const WindowInfo &aidf);
const QDBusArgument &operator>>(const QDBusArgument &a, WindowInfo &aidf);

class WindowStack: public QObject, protected QDBusContext {
public:
	explicit WindowStack(const QDBusConnection &connection,
			QObject *parent = 0);

	virtual ~WindowStack();

public:
	static const QString DBUS_NAME;
	static const QString DBUS_PATH;

	static void registerMetaTypes();

public Q_SLOTS:
	QString GetAppIdFromPid(uint pid);

	QList<WindowInfo> GetWindowStack();

Q_SIGNALS:
	void FocusedWindowChanged(uint windowId, const QString &appId, uint stage);

	void WindowCreated(uint windowId, const QString &appId);

	void WindowDestroyed(uint windowId, const QString &appId);

protected:
	QScopedPointer<WindowStackAdaptor> m_adaptor;

	QDBusConnection m_connection;
};

Q_DECLARE_METATYPE(WindowInfo);

#endif /* WINDOWSTACK_H_ */
