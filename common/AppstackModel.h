/*
 * AppstackModel.h
 *
 *  Created on: 1 Nov 2013
 *      Author: pete
 */

#ifndef HUD_COMMON_APPSTACKMODEL_H_
#define HUD_COMMON_APPSTACKMODEL_H_

#include <common/HudDee.h>

namespace hud {
namespace common {

class AppstackModel: public HudDee {
public:
	AppstackModel(unsigned int id);

	virtual ~AppstackModel();
};

}
}

#endif /* HUD_COMMON_APPSTACKMODEL_H_ */
