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

#include <common/DBusTypes.h>
#include <service/Application.h>
#include <service/ApplicationAdaptor.h>

using namespace hud::common;

namespace hud {
namespace service {

Application::Application(unsigned int id, const QDBusConnection &connection,
		QObject *parent) :
		QObject(parent), m_adaptor(new ApplicationAdaptor(this)), m_connection(
				connection), m_path(DBusTypes::applicationPath(id)) {
	if (!m_connection.registerObject(m_path.path(), this)) {
		throw std::logic_error(_("Unable to register HUD object on DBus"));
	}
}

Application::~Application() {
	m_connection.unregisterObject(m_path.path());
}

}
}
