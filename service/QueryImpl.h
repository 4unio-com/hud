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

#ifndef HUD_SERVICE_QUERYIMPL_H_
#define HUD_SERVICE_QUERYIMPL_H_

#include <common/ResultsModel.h>
#include <common/AppstackModel.h>
#include <service/ApplicationList.h>
#include <service/Query.h>

#include <QDBusContext>
#include <QDBusConnection>
#include <QDBusServiceWatcher>
#include <QDBusVariant>
#include <QScopedPointer>
#include <QSharedPointer>

class QueryAdaptor;

namespace hud {
namespace service {

class HudService;

class Q_DECL_EXPORT QueryImpl: public Query, protected QDBusContext {
Q_OBJECT
public:
	explicit QueryImpl(unsigned int id, const QString &query,
			const QString &sender, HudService &service,
			ApplicationList::Ptr applicationList,
			const QDBusConnection &connection, QObject *parent = 0);

	virtual ~QueryImpl();

	const QDBusObjectPath & path() const;

	const QList<Result> & results() const;

	QString appstackModel() const;

	QString currentQuery() const;

	QString resultsModel() const;

	Q_PROPERTY(QStringList ToolbarItems READ toolbarItems)
	QStringList toolbarItems() const;

public Q_SLOTS:
	void CloseQuery();

	void ExecuteCommand(const QDBusVariant &item, uint timestamp);

	QString ExecuteParameterized(const QDBusVariant &item, uint timestamp,
			QDBusObjectPath &actionPath, QDBusObjectPath &modelPath,
			int &modelSection);

	void ExecuteToolbar(const QString &item, uint timestamp);

	int UpdateApp(const QString &app);

	int UpdateQuery(const QString &query);

	int VoiceQuery(QString &query);

protected Q_SLOTS:
	void serviceUnregistered(const QString &service);

protected:
	void refresh();

	QScopedPointer<QueryAdaptor> m_adaptor;

	QDBusConnection m_connection;

	QDBusObjectPath m_path;

	HudService &m_service;

	ApplicationList::Ptr m_applicationList;

	QString m_query;

	QDBusServiceWatcher m_serviceWatcher;

	QSharedPointer<hud::common::ResultsModel> m_resultsModel;

	QSharedPointer<hud::common::AppstackModel> m_appstackModel;

	QList<Result> m_results;

	WindowToken::Ptr m_windowToken;
};

}
}

#endif /* HUD_SERVICE_QUERYIMPL_H_ */
