/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of version 3 of the GNU Lesser General Public License as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Pete Woods <pete.woods@canonical.com>
 */

#ifndef HUD_COMMON_DBUSTYPES_H_
#define HUD_COMMON_DBUSTYPES_H_

#include <common/NameObject.h>
#include <common/Suggestion.h>

namespace hud {
namespace common {

class DBusTypes {
public:
	static const QString HUD_SERVICE_DBUS_NAME;

	static const QString HUD_SERVICE_DBUS_PATH;

	static void registerMetaTypes();

	static QString queryPath(unsigned int id);

	static QString applicationPath(unsigned int id);
};

}
}

#endif /* HUD_COMMON_DBUSTYPES_H_ */
