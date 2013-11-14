/*
 * RawDBusTransformer.h
 *
 *  Created on: 14 Nov 2013
 *      Author: pete
 */

#ifndef RAWDBUSTRANSFORMER_H_
#define RAWDBUSTRANSFORMER_H_

#include <QVariantMap>
#include <QVariantList>
#include <QDBusArgument>

namespace hud {
namespace testutils {

class RawDBusTransformer {
private:
	explicit RawDBusTransformer();

	virtual ~RawDBusTransformer();

public:
	static void transform(QVariantList &list);

	static void transform(QVariantMap &map);

	static void transform(QVariant &variant);

	static QVariant transform(const QDBusArgument & value);
};

} /* namespace testutils */
} /* namespace hud */
#endif /* RAWDBUSTRANSFORMER_H_ */
