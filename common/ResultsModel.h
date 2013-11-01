/*
 * ResultsModel.h
 *
 *  Created on: 1 Nov 2013
 *      Author: pete
 */

#ifndef HUD_COMMON_RESULTSMODEL_H_
#define HUD_COMMON_RESULTSMODEL_H_

#include <common/HudDee.h>

namespace hud {
namespace common {

class ResultsModel: public HudDee {
public:
	ResultsModel(unsigned int id);

	virtual ~ResultsModel();
};

}
}

#endif /* HUD_COMMON_RESULTSMODEL_H_ */
