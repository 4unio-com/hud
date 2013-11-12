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

#ifndef HUD_SERVICE_RESULT_H_
#define HUD_SERVICE_RESULT_H_

#include <QString>
#include <QList>
#include <QPair>

namespace hud {
namespace service {

class Result {
public:
	explicit Result();

	explicit Result(qulonglong id, const QString &commandName,
			const QList<QPair<int, int>> &commandHighlights,
			const QString &description,
			const QList<QPair<int, int>> &descriptionHighlights,
			const QString &shortcut, int distance, bool parameterized);

	virtual ~Result();

	qulonglong id() const;

	const QString & commandName() const;

	const QList<QPair<int, int>> & commandHighlights() const;

	const QString & description() const;

	const QList<QPair<int, int>> & descriptionHighlights() const;

	const QString & shortcut() const;

	int distance() const;

	bool parameterized() const;

protected:
	uint64_t m_id;

	QString m_commandName;

	QList<QPair<int, int>> m_commandHighlights;

	QString m_description;

	QList<QPair<int, int>> m_descriptionHighlights;

	QString m_shortcut;

	int m_distance;

	bool m_parameterized;
};

}
}
#endif /* HUD_SERVICE_RESULT_H_ */
