/*
 * RawDBusTransformer.cpp
 *
 *  Created on: 14 Nov 2013
 *      Author: pete
 */

#include <testutils/RawDBusTransformer.h>
#include <QDebug>

using namespace hud::testutils;

RawDBusTransformer::RawDBusTransformer() {

}

RawDBusTransformer::~RawDBusTransformer() {
}

QVariant RawDBusTransformer::transform(const QDBusArgument & value) {
	switch (value.currentType()) {
	case QDBusArgument::ArrayType: {
		value.beginArray();
		QVariantList list = transform(value).toList();
		value.endArray();
		return list;
	}
	case QDBusArgument::MapType: {
		QVariantMap map;
		value >> map;
		transform(map);
		return map;
	}
	case QDBusArgument::StructureType: {
		value.beginStructure();
		QVariantList list;
		while (!value.atEnd()) {
			list << value.asVariant();
		}
		value.endStructure();
		return list;
		break;
	}
	default:
		break;
	}

	return QVariant();
}
void RawDBusTransformer::transform(QVariant &variant) {
	if (variant.canConvert<QVariantList>()) {
		QVariantList list = variant.toList();
		transform(list);
		variant = list;
	} else if (variant.canConvert<QDBusArgument>()) {
		QDBusArgument value(variant.value<QDBusArgument>());
		variant = transform(value);
	}
}

void RawDBusTransformer::transform(QVariantMap &map) {
	for (auto it(map.begin()); it != map.end(); ++it) {
		transform(*it);
	}
}

void RawDBusTransformer::transform(QVariantList &list) {
	for (auto it(list.begin()); it != list.end(); ++it) {
		transform(*it);
	}
}
