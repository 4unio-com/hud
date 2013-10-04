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

#ifndef BAMFWINDOWSTACK_H_
#define BAMFWINDOWSTACK_H_

#include <AbstractWindowStack.h>

class BamfWindowStack: public AbstractWindowStack {
public:
	explicit BamfWindowStack(const QDBusConnection &connection,
			QObject *parent = 0);

	virtual ~BamfWindowStack();

public Q_SLOTS:
	virtual QString GetAppIdFromPid(uint pid);

	virtual QList<WindowInfo> GetWindowStack();
};

#endif /* BAMFWINDOWSTACK_H_ */
