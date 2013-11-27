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

#ifndef HUD_SERVICE_WINDOWIMPL_H_
#define HUD_SERVICE_WINDOWIMPL_H_

#include <service/WindowContextImpl.h>
#include <service/DBusMenuCollector.h>
#include <service/GMenuCollector.h>
#include <service/ItemStore.h>

#include <QList>
#include <QSharedPointer>
#include <QTimer>

namespace hud {
namespace service {

class Factory;
class WindowImpl;

class Q_DECL_EXPORT WindowTokenImpl: public WindowToken {
Q_OBJECT

public:
	explicit WindowTokenImpl(const QList<CollectorToken::Ptr> &tokens);

	virtual ~WindowTokenImpl();

	virtual void search(const QString &query, QList<Result> &results);

	virtual void execute(unsigned long long commandId);

	virtual QString executeParameterized(unsigned long long commandId,
			QString &baseAction, QDBusObjectPath &actionPath,
			QDBusObjectPath &modelPath);

	virtual void commands(QList<QStringList>& commandsList);

	virtual const QList<CollectorToken::Ptr> & tokens() const;

protected Q_SLOTS:
	void childChanged();

protected:
	ItemStore m_items;

	QList<CollectorToken::Ptr> m_tokens;

	QTimer m_timer;
};

class WindowImpl: public WindowContextImpl, public Window {
	friend WindowTokenImpl;

public:
	explicit WindowImpl(unsigned int windowId, const QString &applicationId,
			WindowContext::Ptr allWindowsContext, Factory &factory);

	virtual ~WindowImpl();

	virtual WindowToken::Ptr activate();

protected:
	WindowContext::Ptr m_allWindowsContext;

	Collector::Ptr m_dbusMenuCollector;

	Collector::Ptr m_gMenuCollector;

	QWeakPointer<WindowToken> m_windowToken;
};

}
}
#endif /* HUD_SERVICE_WINDOWIMPL_H_ */
