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

#include <BamfWindowStack.h>

BamfWindowStack::BamfWindowStack(const QDBusConnection &connection,
		QObject *parent) :
		AbstractWindowStack(connection, parent) {
}

BamfWindowStack::~BamfWindowStack() {
}

QString BamfWindowStack::GetAppIdFromPid(uint pid) {
	return QString();
}

QList<WindowInfo> BamfWindowStack::GetWindowStack() {
	return QList<WindowInfo>();
}

