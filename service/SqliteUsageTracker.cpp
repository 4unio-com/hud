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

#include <service/SqliteUsageTracker.h>

#include <QDebug>

using namespace hud::service;

SqliteUsageTracker::SqliteUsageTracker() {
}

SqliteUsageTracker::~SqliteUsageTracker() {
}

void SqliteUsageTracker::markUsage(const QString &applicationId,
		const QString &entry) {
	UsagePair pair(applicationId, entry);
	m_usage.find(pair);

	if (m_usage.contains(pair)) {
		++(*m_usage.find(pair));
	} else {
		m_usage[pair] = 1;
	}
}

unsigned int SqliteUsageTracker::usage(const QString &applicationId,
		const QString &entry) const {
	return m_usage[UsagePair(applicationId, entry)];
}
