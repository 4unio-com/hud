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

#include <service/Window.h>
#include <service/DBusMenuCollector.h>
#include <service/GMenuCollector.h>

#include <QSharedPointer>

namespace hud {
namespace service {

class Factory;

class WindowImpl: public Window {
public:
	explicit WindowImpl(unsigned int windowId, const QString &applicationId,
			Factory &factory);

	virtual ~WindowImpl();

	virtual void activate();

protected:
	Collector::Ptr m_dbusMenuCollector;

	Collector::Ptr m_gMenuCollector;

	ItemStore m_items;
};

}
}
#endif /* HUD_SERVICE_WINDOWIMPL_H_ */
