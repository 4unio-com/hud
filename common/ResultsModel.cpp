/*
 * ResultsModel.cpp
 *
 *  Created on: 1 Nov 2013
 *      Author: pete
 */

#include <common/ResultsModel.h>
#include <QString>
#include <glib.h>

typedef enum _HudQueryResultsColumns {
	HUD_QUERY_RESULTS_COMMAND_ID = 0,
	HUD_QUERY_RESULTS_COMMAND_NAME,
	HUD_QUERY_RESULTS_COMMAND_HIGHLIGHTS,
	HUD_QUERY_RESULTS_DESCRIPTION,
	HUD_QUERY_RESULTS_DESCRIPTION_HIGHLIGHTS,
	HUD_QUERY_RESULTS_SHORTCUT,
	HUD_QUERY_RESULTS_DISTANCE,
	HUD_QUERY_RESULTS_PARAMETERIZED,
	/* Last */
	HUD_QUERY_RESULTS_COUNT
} HudQueryResultsColumns;

#define HUD_QUERY_RESULTS_COMMAND_ID_TYPE "v"
#define HUD_QUERY_RESULTS_COMMAND_NAME_TYPE "s"
#define HUD_QUERY_RESULTS_COMMAND_HIGHLIGHTS_TYPE "a(ii)"
#define HUD_QUERY_RESULTS_DESCRIPTION_TYPE "s"
#define HUD_QUERY_RESULTS_DESCRIPTION_HIGHLIGHTS_TYPE "a(ii)"
#define HUD_QUERY_RESULTS_SHORTCUT_TYPE "s"
#define HUD_QUERY_RESULTS_DISTANCE_TYPE "u"
#define HUD_QUERY_RESULTS_PARAMETERIZED_TYPE "b"

/* Schema that is used in the DeeModel representing
 the results */
static const char * results_model_schema[HUD_QUERY_RESULTS_COUNT] = {
HUD_QUERY_RESULTS_COMMAND_ID_TYPE,
HUD_QUERY_RESULTS_COMMAND_NAME_TYPE,
HUD_QUERY_RESULTS_COMMAND_HIGHLIGHTS_TYPE,
HUD_QUERY_RESULTS_DESCRIPTION_TYPE,
HUD_QUERY_RESULTS_DESCRIPTION_HIGHLIGHTS_TYPE,
HUD_QUERY_RESULTS_SHORTCUT_TYPE,
HUD_QUERY_RESULTS_DISTANCE_TYPE,
HUD_QUERY_RESULTS_PARAMETERIZED_TYPE, };

const QString RESULTS_FORMAT_STRING("com.canonical.hud.query%1.results");

using namespace hud::common;

ResultsModel::ResultsModel(unsigned int id) :
		HudDee(RESULTS_FORMAT_STRING.arg(id).toStdString()) {

	setSchema(results_model_schema, G_N_ELEMENTS(results_model_schema));
}

ResultsModel::~ResultsModel() {
}

void ResultsModel::addResult(qulonglong id, const QString &commandName,
		const QList<QPair<int, int>> &commandHighlights,
		const QString &description,
		const QList<QPair<int, int>> &descriptionHighlights,
		const QString &shortcut, int distance, bool parameterized) {

	GVariant *actionh = NULL;
	if (commandHighlights.isEmpty()) {
		actionh = g_variant_new_array(G_VARIANT_TYPE("(ii)"), NULL, 0);
	} else {
		GVariantBuilder builder;
		g_variant_builder_init(&builder, G_VARIANT_TYPE("a(ii)"));
		for (const QPair<int, int> &highlight : commandHighlights) {
			g_variant_builder_add(&builder, "(ii)", highlight.first,
					highlight.second);
		}
		actionh = g_variant_builder_end(&builder);
	}

	GVariant *desch = NULL;
	if (descriptionHighlights.isEmpty()) {
		desch = g_variant_new_array(G_VARIANT_TYPE("(ii)"), NULL, 0);
	} else {
		GVariantBuilder builder;
		g_variant_builder_init(&builder, G_VARIANT_TYPE("a(ii)"));
		for (const QPair<int, int> &highlight : descriptionHighlights) {
			g_variant_builder_add(&builder, "(ii)", highlight.first,
					highlight.second);
		}
		desch = g_variant_builder_end(&builder);
	}

	GVariant *columns[HUD_QUERY_RESULTS_COUNT + 1];
	columns[HUD_QUERY_RESULTS_COMMAND_ID] = g_variant_new_variant(
			g_variant_new_uint64(id));
	columns[HUD_QUERY_RESULTS_COMMAND_NAME] = g_variant_new_string(
			commandName.toUtf8().data());
	columns[HUD_QUERY_RESULTS_COMMAND_HIGHLIGHTS] = actionh;
	columns[HUD_QUERY_RESULTS_DESCRIPTION] = g_variant_new_string(
			description.toUtf8().data());
	columns[HUD_QUERY_RESULTS_DESCRIPTION_HIGHLIGHTS] = desch;
	columns[HUD_QUERY_RESULTS_SHORTCUT] = g_variant_new_string(
			shortcut.toUtf8().data());
	columns[HUD_QUERY_RESULTS_DISTANCE] = g_variant_new_uint32(distance);
	columns[HUD_QUERY_RESULTS_PARAMETERIZED] = g_variant_new_boolean(
			parameterized);
	columns[HUD_QUERY_RESULTS_COUNT] = NULL;

	appendRow(columns);
}
