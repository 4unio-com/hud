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

#include <common/Action.h>

#include <QDBusMetaType>

using namespace hud::common;

QDBusArgument & operator<<(QDBusArgument &argument, const Action &action) {
	argument.beginStructure();
//	argument << suggestion.m_description << suggestion.m_icon
//			<< suggestion.m_unknown1 << suggestion.m_unknown2
//			<< suggestion.m_unknown3 << QDBusVariant(suggestion.m_id);
	argument.endStructure();
	return argument;
}

const QDBusArgument & operator>>(const QDBusArgument &argument,
		Action &action) {
	argument.beginStructure();
//	argument >> suggestion.m_description >> suggestion.m_icon
//			>> suggestion.m_unknown1 >> suggestion.m_unknown2
//			>> suggestion.m_unknown3 >> suggestion.m_id;
	argument.endStructure();
	return argument;
}

Action::Action() {

}

Action::~Action() {
}

void Action::registerMetaTypes() {
	qRegisterMetaType<Action>();
	qDBusRegisterMetaType<Action>();

	qRegisterMetaType<QList<Action>>();
	qDBusRegisterMetaType<QList<Action>>();
}
