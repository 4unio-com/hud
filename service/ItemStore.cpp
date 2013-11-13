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

#include <service/ItemStore.h>

#include <columbus.hh>
#include <QRegularExpression>
#include <QDebug>

using namespace hud::service;
using namespace Columbus;

static const QRegularExpression SINGLE_AMPERSAND("(?<![&])[&](?![&])");
static const QRegularExpression WHITESPACE("\\s+");

ItemStore::ItemStore() :
		m_nextId(0) {
}

ItemStore::~ItemStore() {
}

void ItemStore::indexMenu(const QMenu *menu, const QStringList &stack) {
	for (const QAction *action : menu->actions()) {
		if (!action->isEnabled()) {
			continue;
		}
		if (action->isSeparator()) {
			continue;
		}

		QStringList text(
				action->text().remove(SINGLE_AMPERSAND).replace("&&", "&").split(
						WHITESPACE));

		QMenu *child(action->menu());
		if (child) {
			QStringList childStack(stack);
			childStack << text;
			indexMenu(child, childStack);
		} else {
			Result::HighlightList commandHighlights;
			commandHighlights << Result::Highlight(2, 3);

			Result::HighlightList descriptionHighlights;
			descriptionHighlights << Result::Highlight(3, 5);

			Document document(m_nextId);

			WordList command;
			for (const QString &word : text) {
				command.addWord(Word(word.toStdString()));
			}
			document.addText(Word("command"), command);
			qDebug() << text;

			WordList wordList;
			for (const QString &word : stack) {
				wordList.addWord(Word(word.toStdString()));
			}
			document.addText(Word("context"), wordList);

			m_corpus.addDocument(document);
			m_actions[m_nextId] = action;

			++m_nextId;
		}
	}
}

void ItemStore::indexMenu(const QMenu *menu) {
	indexMenu(menu, QStringList());
	m_matcher.index(m_corpus);
}

void ItemStore::search(const QString &query, QList<Result> &results) {
	WordList queryList;
	for (const QString &word : query.split(WHITESPACE)) {
		queryList.addWord(word.toStdString());
	}

	MatchResults matchResults(m_matcher.match(queryList));

	for (size_t i(0); i < matchResults.size(); ++i) {
		DocumentID id(matchResults.getDocumentID(i));
		double relevancy(matchResults.getRelevancy(i));

		const QAction *action = m_actions[id];
		results
				<< Result(id,
						action->text().remove(SINGLE_AMPERSAND).replace("&&",
								"&"), Result::HighlightList(), QString(),
						Result::HighlightList(), QString(), relevancy * 100,
						false);

	}

}
