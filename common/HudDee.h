/*
 * HudDee.h
 *
 *  Created on: 1 Nov 2013
 *      Author: pete
 */

#ifndef HUD_COMMON_HUDDEE_H_
#define HUD_COMMON_HUDDEE_H_

#include <memory>

namespace hud {
namespace common {

class HudDee {
public:
	HudDee(const std::string &resultsName);

	virtual ~HudDee();

	const std::string & name() const;

protected:
	void setSchema(const char* const *columnSchemas, unsigned int numColumns);

	class Priv;

	std::shared_ptr<Priv> p;
};

}
}

#endif /* HUD_COMMON_HUDDEE_H_ */
