/*
 * ResultsModel.h
 *
 *  Created on: 1 Nov 2013
 *      Author: pete
 */

#ifndef HUD_COMMON_RESULTSMODEL_H_
#define HUD_COMMON_RESULTSMODEL_H_

#include <common/HudDee.h>

#include <QList>
#include <QPair>
#include <QString>

namespace hud {
namespace common {

class ResultsModel: public HudDee {
public:
	ResultsModel(unsigned int id);

	virtual ~ResultsModel();

	void addResult(unsigned int id, const QString &command,
			const QList<QPair<int, int>> highlights, const QString &description,
			const QList<QPair<int, int>> descriptionHighlights,
			const QString &shortcut, int distance, bool parameterized);
};

}
}

#endif /* HUD_COMMON_RESULTSMODEL_H_ */
