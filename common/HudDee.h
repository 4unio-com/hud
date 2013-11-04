/*
 * HudDee.h
 *
 *  Created on: 1 Nov 2013
 *      Author: pete
 */

#ifndef HUD_COMMON_HUDDEE_H_
#define HUD_COMMON_HUDDEE_H_

#include <memory>

typedef struct _GVariant GVariant;

typedef int (*CompareRowFunc)(GVariant** row1, GVariant** row2,
		void* user_data);

namespace hud {
namespace common {

class HudDee {
public:
	HudDee(const std::string &resultsName);

	virtual ~HudDee();

	const std::string & name() const;

protected:
	void setSchema(const char* const *columnSchemas, unsigned int numColumns);

	void beginChangeset();

	void appendRow(GVariant **row_members);

	void insertRowSorted(GVariant **row_members, CompareRowFunc cmp_func);

	void endChangeset();

	class Priv;

	std::shared_ptr<Priv> p;
};

}
}

#endif /* HUD_COMMON_HUDDEE_H_ */
