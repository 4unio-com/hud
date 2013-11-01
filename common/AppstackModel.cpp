/*
 * AppstackModel.cpp
 *
 *  Created on: 1 Nov 2013
 *      Author: pete
 */

#include <common/AppstackModel.h>

#include <QString>
#include <glib.h>

using namespace hud::common;

typedef enum {
	HUD_QUERY_APPSTACK_APPLICATION_ID = 0,
	HUD_QUERY_APPSTACK_ICON_NAME,
	HUD_QUERY_APPSTACK_ITEM_TYPE,
	/* Last */
	HUD_QUERY_APPSTACK_COUNT
} HudQueryAppstackColumns;

#define HUD_QUERY_APPSTACK_APPLICATION_ID_TYPE "s"
#define HUD_QUERY_APPSTACK_ICON_NAME_TYPE "s"
#define HUD_QUERY_APPSTACK_ITEM_TYPE_TYPE "i"

/* Schema that is used in the DeeModel representing
 the appstack */
static const gchar * appstack_model_schema[HUD_QUERY_APPSTACK_COUNT] = {
HUD_QUERY_APPSTACK_APPLICATION_ID_TYPE,
HUD_QUERY_APPSTACK_ICON_NAME_TYPE,
HUD_QUERY_APPSTACK_ITEM_TYPE_TYPE, };

const QString APPSTACK_FORMAT_STRING("com.canonical.hud.query%1.appstack");

AppstackModel::AppstackModel(unsigned int id) :
		HudDee(APPSTACK_FORMAT_STRING.arg(id).toStdString()) {
	setSchema(appstack_model_schema, G_N_ELEMENTS(appstack_model_schema));
}

AppstackModel::~AppstackModel() {
}
