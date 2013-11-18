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

#ifndef HUD_SERVICE_ITEMSTORE_H_
#define HUD_SERVICE_ITEMSTORE_H_

#include <service/Result.h>

#include <QSharedPointer>
#include <QMenu>
#include <QStringList>
#include <Corpus.hh>
#include <Matcher.hh>

namespace hud {
namespace service {

class ItemStore {
public:
	typedef QSharedPointer<ItemStore> Ptr;

	explicit ItemStore();

	virtual ~ItemStore();

	void indexMenu(const QMenu *menu);

	void search(const QString &query, QList<Result> &results);

	void execute(unsigned long long commandId, uint timestamp);

protected:
	void indexMenu(const QMenu *menu, const QStringList &stack);

	Columbus::Corpus m_corpus;

	Columbus::Matcher m_matcher;

	DocumentID m_nextId;

	QMap<DocumentID, QAction *> m_actions;
};

}
}
#endif /* HUD_SERVICE_ITEMSTORE_H_ */
