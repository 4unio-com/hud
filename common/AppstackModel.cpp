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

	addApplication("test-app-1", "icon 1", HUD_SOURCE_ITEM_TYPE_FOCUSED_APP);
	addApplication("test-app-2", "icon 2", HUD_SOURCE_ITEM_TYPE_BACKGROUND_APP);
	flush();
}

AppstackModel::~AppstackModel() {
}

/* Sort function for the appstack */
static gint appstack_sort(GVariant **row1, GVariant **row2,
		gpointer user_data) {
	Q_UNUSED(user_data);

	gint32 type1 = g_variant_get_int32(row1[HUD_QUERY_APPSTACK_ITEM_TYPE]);
	gint32 type2 = g_variant_get_int32(row2[HUD_QUERY_APPSTACK_ITEM_TYPE]);

	/* If the types are the same, we'll sort by ID */
	if (type1 == type2) {
		const gchar * app_id1 = g_variant_get_string(
				row1[HUD_QUERY_APPSTACK_APPLICATION_ID], NULL);
		const gchar * app_id2 = g_variant_get_string(
				row2[HUD_QUERY_APPSTACK_APPLICATION_ID], NULL);

		return g_strcmp0(app_id1, app_id2);
	}

	return type1 - type2;
}

void AppstackModel::addApplication(const QString &applicationId,
		const QString &iconName, ItemType itemType) {
	GVariant * columns[HUD_QUERY_APPSTACK_COUNT + 1];
	columns[HUD_QUERY_APPSTACK_APPLICATION_ID] = g_variant_new_string(
			applicationId.toUtf8().data());
	columns[HUD_QUERY_APPSTACK_ICON_NAME] = g_variant_new_string(
			iconName.toUtf8().data());
	columns[HUD_QUERY_APPSTACK_ITEM_TYPE] = g_variant_new_int32(itemType);
	columns[HUD_QUERY_APPSTACK_COUNT] = NULL;

	insertRowSorted(columns, appstack_sort);
}
