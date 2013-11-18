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

#ifndef HUD_SERVICE_WINDOW_H_
#define HUD_SERVICE_WINDOW_H_

#include <service/Result.h>

#include <QList>
#include <QSharedPointer>

namespace hud {
namespace service {

class WindowToken {
public:
	typedef QSharedPointer<WindowToken> Ptr;

	virtual ~WindowToken();

	virtual void search(const QString &query, QList<Result> &results) = 0;

	virtual void execute(unsigned long long commandId, uint timestamp) = 0;

protected:
	explicit WindowToken();
};

class Window {
public:
	typedef QSharedPointer<Window> Ptr;

	explicit Window();

	virtual ~Window();

	virtual WindowToken::Ptr activate() = 0;

	virtual void setContext(const QString &context) = 0;
};

}
}
#endif /* HUD_SERVICE_WINDOW_H_ */
