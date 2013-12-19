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

#include <common/Localisation.h>
#include <service/ItemStore.h>
#include <QGSettings/qgsettings.h>

#include <columbus.hh>
#include <QRegularExpression>
#include <QDebug>
#include <QDBusObjectPath>
#include <algorithm>

using namespace hud::service;
using namespace Columbus;

static const QRegularExpression SINGLE_AMPERSAND("(?<![&])[&](?![&])");
static const QRegularExpression WHITESPACE("\\s+");
static const QRegularExpression WHITESPACE_OR_SEMICOLON("[;\\s+]");

ItemStore::ItemStore(const QString &applicationId,
		UsageTracker::Ptr usageTracker) :
		m_applicationId(applicationId), m_usageTracker(usageTracker), m_nextId(
				0) {
	ErrorValues &errorValues(m_matcher.getErrorValues());
	errorValues.addStandardErrors();

	if (qEnvironmentVariableIsSet("HUD_IGNORE_SEARCH_SETTINGS")) {
		errorValues.setInsertionError(100);
		errorValues.setDeletionError(100);
		errorValues.setEndDeletionError(1);
		errorValues.setTransposeError(15);
	} else {
		m_settings.reset(
				new QGSettings("com.canonical.indicator.appmenu.hud.search",
						"/com/canonical/indicator/appmenu/hud/search/"));
		connect(m_settings.data(), SIGNAL(changed(const QString &)), this,
				SLOT(settingChanged(const QString &)));
		settingChanged(QString());
	}
}

ItemStore::~ItemStore() {
}

void ItemStore::settingChanged(const QString &key) {
	Q_UNUSED(key);

	ErrorValues &errorValues(m_matcher.getErrorValues());
	errorValues.setInsertionError(m_settings->get("addPenalty").toUInt() * 10);
	errorValues.setDeletionError(m_settings->get("dropPenalty").toUInt() * 10);
	errorValues.setEndDeletionError(m_settings->get("endDropPenalty").toUInt());
	errorValues.setTransposeError(m_settings->get("swapPenalty").toUInt());
}

static QString convertActionText(const QAction *action) {
	return action->text().remove(SINGLE_AMPERSAND).replace("&&", "&");
}

void ItemStore::indexMenu(const QMenu *menu, const QMenu *root,
		const QStringList &stack, const QList<int> &index) {
	int i(-1);
	for (QAction *action : menu->actions()) {
		++i;

		if (!action->isEnabled()) {
			continue;
		}
		if (action->isSeparator()) {
			continue;
		}

		QStringList text(convertActionText(action).split(WHITESPACE));

		bool isParameterized(action->property("isParameterized").toBool());

		// We don't descend into parameterized actions
		QMenu *child(action->menu());
		if (!isParameterized && child) {
			QStringList childStack(stack);
			childStack << text;
			QList<int> childIndex(index);
			childIndex << i;
			indexMenu(child, root, childStack, childIndex);
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

			WordList wordList;
			QVariant keywords(action->property("keywords"));
			QStringList context;
			if (!keywords.isNull()) {
				context = keywords.toString().split(WHITESPACE_OR_SEMICOLON);
			} else {
				context = stack;
			}
			for (const QString &word : context) {
				wordList.addWord(Word(word.toStdString()));
			}
			document.addText(Word("context"), wordList);

			m_corpus.addDocument(document);
			Item::Ptr item(new Item(root, index, i));
			m_items[m_nextId] = item;

			QVariant toolbarItem(action->property("hud-toolbar-item"));
			if (!toolbarItem.isNull()) {
				m_toolbarItems[toolbarItem.toString()] = item;
			}

			++m_nextId;
		}
	}
}

void ItemStore::indexMenu(const QMenu *menu) {
	if (menu == nullptr) {
		return;
	}
	indexMenu(menu, menu, QStringList(), QList<int>());
	m_matcher.index(m_corpus);
}

static void findHighlights(Result::HighlightList &highlights,
		const QStringMatcher &matcher, int length, const QString &s) {

	if (length > 0) {
		int idx = matcher.indexIn(s);
		while (idx != -1) {
			highlights << Result::Highlight(idx, idx + length);
			idx = matcher.indexIn(s, idx + length);
		}
	}
}

static QString convertToEntry(Item::Ptr item, const QAction *action) {
	QString result;
	bool first(true);
	for (const QAction *context : item->context()) {
		if (first) {
			first = false;
		} else {
			result.append("||");
		}
		result.append(context->text());
	}
	result.append("||");
	result.append(action->text());
	return result;
}

void ItemStore::search(const QString &query, QList<Result> &results) {
	QStringMatcher stringMatcher(query, Qt::CaseInsensitive);

	if (query.isEmpty()) {
		QMap<unsigned int, DocumentID> tempResults;

		for (auto it(m_items.constBegin()); it != m_items.constEnd(); ++it) {
			tempResults.insertMulti(m_usageTracker->usage(m_applicationId,
					convertToEntry(it.value(), it.value()->action())), it.key());
		}

		int maxResults = std::min(m_items.size(), 20);
		int count = 0;
		QMapIterator<unsigned int, DocumentID> it(tempResults);
		it.toBack();
		while (count < maxResults && it.hasPrevious()) {
			it.previous();
			addResult(it.value(), stringMatcher, 0, 0, results);
			++count;
		}

	} else {
		WordList queryList;
		for (const QString &word : query.split(WHITESPACE)) {
			queryList.addWord(word.toStdString());
		}

		MatchResults matchResults(m_matcher.match(queryList));

		int queryLength(query.length());

		size_t maxResults = std::min(matchResults.size(), size_t(20));

		for (size_t i(0); i < maxResults; ++i) {
			DocumentID id(matchResults.getDocumentID(i));
			double relevancy(matchResults.getRelevancy(i));
			addResult(id, stringMatcher, queryLength, relevancy, results);
		}
	}

}

void ItemStore::addResult(DocumentID id, const QStringMatcher &stringMatcher,
		const int queryLength, const double relevancy, QList<Result> &results) {

	Item::Ptr item(m_items[id]);
	const QAction *action(item->action());

	QString commandName(convertActionText(action));

	Result::HighlightList commandHighlights;
	findHighlights(commandHighlights, stringMatcher, queryLength, commandName);

	QString description;
	QVariant keywords(action->property("keywords"));
	if (!keywords.isNull()) {
		description = keywords.toString().replace(";", _(", "));
	} else {
		bool first(true);
		for (const QAction *a : item->context()) {
			if (first) {
				first = false;
			} else {
				description.append(_(", "));
			}
			description.append(convertActionText(a));
		}
	}

	Result::HighlightList descriptionHighlights;
	findHighlights(descriptionHighlights, stringMatcher, queryLength,
			description);

	bool isParameterized(action->property("isParameterized").toBool());

	results
			<< Result(id, commandName, commandHighlights, description,
					descriptionHighlights, action->shortcut().toString(),
					relevancy * 100, isParameterized);

}

void ItemStore::executeItem(Item::Ptr item) {
	if (item.isNull()) {
		qWarning() << "Tried to execute unknown command";
		return;
	}

	QAction *action(item->action());

	if (action == nullptr) {
		qWarning() << "Tried to execute unknown command";
		return;
	}

	action->activate(QAction::ActionEvent::Trigger);
	m_usageTracker->markUsage(m_applicationId, convertToEntry(item, action));
}

void ItemStore::execute(unsigned long long int commandId) {
	Item::Ptr item(m_items[commandId]);
	executeItem(item);
}

QString ItemStore::executeParameterized(unsigned long long commandId,
		QString &baseAction, QDBusObjectPath &actionPath,
		QDBusObjectPath &modelPath) {

	Item::Ptr item(m_items[commandId]);
	if (item.isNull()) {
		qWarning() << "Tried to execute unknown parameterized command"
				<< commandId;
		return QString();
	}

	QAction *action(item->action());

	if (action == nullptr) {
		qWarning() << "Tried to execute unknown parameterized command"
				<< commandId;
		return QString();
	}

	baseAction = action->property("actionName").toString();
	actionPath = QDBusObjectPath(action->property("actionsPath").toString());
	modelPath = QDBusObjectPath(action->property("menuPath").toString());

	m_usageTracker->markUsage(m_applicationId, convertToEntry(item, action));
	return action->property("busName").toString();
}

void ItemStore::executeToolbar(const QString &name) {
	executeItem(m_toolbarItems[name]);
}

QList<QStringList> ItemStore::commands() const {
	QList<QStringList> commandsList;

	for (uint i = 0; i < m_corpus.size(); ++i) {
		QStringList command;

		const WordList& words = m_corpus.getDocument(i).getText(
				Word("command"));

		for (uint j = 0; j < words.size(); ++j) {
			command.append(words[j].asUtf8().c_str());
		}

		commandsList.append(command);
	}

	return commandsList;
}

QStringList ItemStore::toolbarItems() const {
	return m_toolbarItems.keys();
}
