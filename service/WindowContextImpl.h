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

#ifndef HUD_SERVICE_WINDOWCONTEXTIMPL_H_
#define HUD_SERVICE_WINDOWCONTEXTIMPL_H_

#include <service/WindowContext.h>

#include <QSharedPointer>

namespace hud {
namespace service {

class Factory;

class WindowContextImpl: public virtual WindowContext {

public:
	explicit WindowContextImpl(Factory &factory);

	virtual ~WindowContextImpl();

	virtual void setContext(const QString &context);

	virtual void addAction(const QString &context, const QString &prefix,
			const QDBusObjectPath &path);

	virtual void addModel(const QString &context, const QDBusObjectPath &path);

protected:
	QString m_context;

	QMap<QString, QString> m_foo;
};

}
}
#endif /* HUD_SERVICE_WINDOWCONTEXTIMPL_H_ */
