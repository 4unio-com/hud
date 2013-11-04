/*
 * AppstackModel.h
 *
 *  Created on: 1 Nov 2013
 *      Author: pete
 */

#ifndef HUD_COMMON_APPSTACKMODEL_H_
#define HUD_COMMON_APPSTACKMODEL_H_

#include <common/HudDee.h>

#include <QString>

namespace hud {
namespace common {

class AppstackModel: public HudDee {
public:
	typedef enum {
		ITEM_TYPE_FOCUSED_APP,
		ITEM_TYPE_SIDESTAGE_APP,
		ITEM_TYPE_BACKGROUND_APP,
		ITEM_TYPE_INDICATOR,
	} ItemType;

	AppstackModel(unsigned int id);

	virtual ~AppstackModel();

	void addApplication(const QString &applicationId, const QString &iconName,
			ItemType itemType);
protected:
};

}
}

#endif /* HUD_COMMON_APPSTACKMODEL_H_ */
