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

#ifndef HUD_COMMON_ACTIONGROUP_H_
#define HUD_COMMON_ACTIONGROUP_H_

#include <QObject>
#include <QDBusArgument>

namespace hud {
namespace common {

class ActionGroup {
public:
	ActionGroup();

	virtual ~ActionGroup();

	static void registerMetaTypes();

	//FIXME Give these proper names

	QVariant m_variant;

	QString m_string;

	QDBusObjectPath m_object;
};

}
}

Q_DECL_EXPORT
QDBusArgument &operator<<(QDBusArgument &argument,
		const hud::common::ActionGroup &actionGroup);

Q_DECL_EXPORT
const QDBusArgument &operator>>(const QDBusArgument &argument,
		hud::common::ActionGroup &actionGroup);

Q_DECLARE_METATYPE(hud::common::ActionGroup)

#endif /* HUD_COMMON_ACTIONGROUP_H_ */
