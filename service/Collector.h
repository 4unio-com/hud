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

#ifndef HUD_SERVICE_COLLECTOR_H_
#define HUD_SERVICE_COLLECTOR_H_

#include <QObject>
#include <QSharedPointer>
#include <QMenu>

#include <service/Result.h>

namespace hud {
namespace service {

class Collector;
class DBusMenuCollector;
class GMenuCollector;

class CollectorToken {
	friend Collector;
	friend DBusMenuCollector;
	friend GMenuCollector;

public:
	typedef QSharedPointer<CollectorToken> Ptr;

	virtual ~CollectorToken();

protected:
	explicit CollectorToken(Collector &collector);

	Collector &m_collector;
};

class Collector: public QObject {
Q_OBJECT

	friend CollectorToken;

public:
	typedef QSharedPointer<Collector> Ptr;

	explicit Collector(QObject *parent = 0);

	virtual ~Collector();

	virtual bool isValid() const = 0;

	virtual CollectorToken::Ptr activate() = 0;

	virtual void search(const QString &query, QList<Result> &results) = 0;

protected:
	virtual void deactivate() = 0;
};

}
}

#endif /* HUD_SERVICE_COLLECTOR_H_ */
