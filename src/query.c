/*
 * Copyright © 2012 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Ryan Lortie <desrt@desrt.ca>
 */

#define G_LOG_DOMAIN "hudquery"

#include <dee.h>

#include "query.h"
#include "hud-query-iface.h"
#include "source-list.h"
#include "result.h"
#include "menumodel-collector.h"
#include "dbusmenu-collector.h"
#include "voice.h"
#include "application-source.h"
#include "application-list.h"
#include "query-columns.h"
#include "watchdog.h"

/**
 * SECTION:hudquery
 * @title: HudQuery
 * @short_description: a stateful query against a #HudSource
 *
 * #HudQuery is a stateful query for a particular search string against
 * a given #HudSource.
 *
 * The query monitors its source for the "change" signal and re-submits
 * the query when changes are reported.  The query has its own change
 * signal that is fired when this happens.
 *
 * The query maintains a list of results from the search which are
 * sorted by relevance and accessible by index.  Contrast this with the
 * stateless nature of #HudSource.
 **/

/**
 * HudQuery:
 *
 * This is an opaque structure type.
 **/

struct _HudQuery
{
  GObject parent_instance;

  HudWatchdog * watchdog;
  HudSource *all_sources;
  HudApplicationList *app_list;
  HudSource *current_source;
  HudSource *last_used_source;
  gchar *search_string;
  HudTokenList *token_list;
  gint num_results;
  guint refresh_id;

  guint querynumber; /* Incrementing count, which one were we? */
  HudQueryIfaceComCanonicalHudQuery * skel;
  gchar * object_path;

  DeeModel * results_model;
  gchar * results_name;
  DeeModelTag * results_tag;

  DeeModel * appstack_model;
  gchar * appstack_name;

  GSequence * results_list; /* Should almost always be NULL except when refreshing the query */
  guint max_usage; /* Used to make the GList search easier */

  HudVoice *voice;
};

typedef GObjectClass HudQueryClass;

G_DEFINE_TYPE (HudQuery, hud_query, G_TYPE_OBJECT)

static guint hud_query_changed_signal;

/* Schema that is used in the DeeModel representing
   the results */
static const gchar * results_model_schema[HUD_QUERY_RESULTS_COUNT] = {
	"v", /* Command ID */
	"s", /* Command Name */
	"a(ii)", /* Highlights in command name */
	"s", /* Description */
	"a(ii)", /* Highlights in description */
	"s", /* Shortcut */
	"u", /* Distance */
	"b", /* Parameterized */
};

/* Schema that is used in the DeeModel representing
   the appstack */
static const gchar * appstack_model_schema[HUD_QUERY_APPSTACK_COUNT] = {
	"s", /* Application ID */
	"s", /* Icon Name */
	"i", /* Item Type */
};

static gint
compare_func (gconstpointer a, gconstpointer b, gpointer user_data)
{
  gint max_usage = ((HudQuery *) user_data)->max_usage;
  return hud_result_get_distance ((HudResult *) a, max_usage)
      - hud_result_get_distance ((HudResult *) b, max_usage);
}

/* Add a HudResult to the list of results. This is inserted at the
   end for the moment, until action sort priorities are implemented.
   We can't sort until we have the max usage, then we can sort
   everything depending on distance and usage.  */
static void
results_list_populate (HudResult * result, gpointer user_data)
{
	HudQuery * query = (HudQuery *)user_data;
	// TODO: Change back to prepend after we have action sort priorities
	// g_sequence_insert_before(g_sequence_get_begin_iter(query->results_list), result);
	g_sequence_append(query->results_list, result);
	return;
}

/* A structure to track the items in the appstack */
typedef struct _appstack_item_t appstack_item_t;
struct _appstack_item_t {
	gchar * app_id;
	gchar * app_icon;
	HudSourceItemType type;
};

/* Free all of them items */
static void
appstack_item_free (gpointer user_data)
{
	appstack_item_t * item = (appstack_item_t *)user_data;

	g_free(item->app_id);
	g_free(item->app_icon);

	g_free(item);
	return;
}

/* Sort function for the appstack */
static gint
appstack_sort (GVariant ** row1, GVariant ** row2, gpointer user_data)
{
	gint32 type1 = g_variant_get_int32(row1[HUD_QUERY_APPSTACK_ITEM_TYPE]);
	gint32 type2 = g_variant_get_int32(row2[HUD_QUERY_APPSTACK_ITEM_TYPE]);

	/* If the types are the same, we'll alphabetize by ID */
	if (type1 == type2) {
		const gchar * app_id1 = g_variant_get_string(row1[HUD_QUERY_APPSTACK_APPLICATION_ID], NULL);
		const gchar * app_id2 = g_variant_get_string(row2[HUD_QUERY_APPSTACK_APPLICATION_ID], NULL);

		return g_strcmp0(app_id1, app_id2);
	}

	return type1 - type2;
}

/* Takes the hash and puts it into the Dee Model */
static void
appstack_hash_to_model (GHashTable * hash, DeeModel * model)
{
	GList * values = g_hash_table_get_values(hash);
	GList * value;

	for (value = values; value != NULL; value = g_list_next(value)) {
		appstack_item_t * item = (appstack_item_t *)value->data;

		GVariant * columns[HUD_QUERY_APPSTACK_COUNT + 1];
		columns[HUD_QUERY_APPSTACK_APPLICATION_ID] = g_variant_new_string(item->app_id ? item->app_id : "");
		columns[HUD_QUERY_APPSTACK_ICON_NAME]      = g_variant_new_string(item->app_icon ? item->app_icon : "");
		columns[HUD_QUERY_APPSTACK_ITEM_TYPE]      = g_variant_new_int32(item->type);
		columns[HUD_QUERY_APPSTACK_COUNT]          = NULL;

		dee_model_insert_row_sorted(model, columns, appstack_sort, NULL);
	}

	return;
}

/* Add an item to the hash table */
static void
appstack_hash_add_source (GHashTable * table, HudSource * source, HudSourceItemType in_type)
{
	if (source == NULL) {
		return;
	}

	const gchar * id = hud_source_get_app_id(source);;
	g_return_if_fail(id != NULL);

	const gchar * icon = hud_source_get_app_icon(source);;
	HudSourceItemType type = in_type;

	if (HUD_IS_MENU_MODEL_COLLECTOR(source) || HUD_IS_DBUSMENU_COLLECTOR(source)) {
		type = HUD_SOURCE_ITEM_TYPE_INDICATOR;
	}

	appstack_item_t * item = g_new0(appstack_item_t, 1);
	item->app_id = g_strdup(id);
	item->app_icon = g_strdup(icon);
	item->type = type;

	g_hash_table_insert(table, g_strdup(id), item);

	return;
}

/* Add a HudItem to the list of app results */
static void
app_results_list_populate (const gchar *application_id, const gchar *application_icon, HudSourceItemType type, gpointer user_data)
{
	g_return_if_fail(application_id != NULL);

	GHashTable * table = (GHashTable *)user_data;

	appstack_item_t * item = g_new0(appstack_item_t, 1);
	item->app_id = g_strdup(application_id);
	item->app_icon = g_strdup(application_icon);
	item->type = type;

	g_hash_table_insert(table, g_strdup(application_id), item);

	return;
}

/* Go through the list and find the item with the highest usage
   that the others will be compared against */
static void
results_list_max_usage (gpointer data, gpointer user_data)
{
	HudResult * result = (HudResult *)data;
	HudItem * item = hud_result_get_item(result);
	guint * max_usage = (guint *)user_data;

	*max_usage = MAX(*max_usage, hud_item_get_usage(item));
	return;
}

/* Turn the results list into a DeeModel. It assumes the results are already sorted. */
static void
results_list_to_model (gpointer data, gpointer user_data)
{
	HudResult * result = (HudResult *)data;
	HudQuery * query = (HudQuery *)user_data;
	HudItem * item = hud_result_get_item(result);

	GVariant * columns[HUD_QUERY_RESULTS_COUNT + 1];
	columns[HUD_QUERY_RESULTS_COMMAND_ID]             = g_variant_new_variant(g_variant_new_uint64(hud_item_get_id(item)));
	columns[HUD_QUERY_RESULTS_COMMAND_NAME]           = g_variant_new_string(hud_item_get_command(item));
	columns[HUD_QUERY_RESULTS_COMMAND_HIGHLIGHTS]     = g_variant_new_array(G_VARIANT_TYPE("(ii)"), NULL, 0);
	columns[HUD_QUERY_RESULTS_DESCRIPTION]            = g_variant_new_string(hud_item_get_description(item));
	columns[HUD_QUERY_RESULTS_DESCRIPTION_HIGHLIGHTS] = g_variant_new_array(G_VARIANT_TYPE("(ii)"), NULL, 0);
	columns[HUD_QUERY_RESULTS_SHORTCUT]               = g_variant_new_string(hud_item_get_shortcut(item));
	columns[HUD_QUERY_RESULTS_DISTANCE]               = g_variant_new_uint32(hud_result_get_distance(result, query->max_usage));
	columns[HUD_QUERY_RESULTS_PARAMETERIZED]          = g_variant_new_boolean(HUD_IS_MODEL_ITEM(item) ? hud_model_item_is_parameterized(HUD_MODEL_ITEM(item)) : FALSE);
	columns[HUD_QUERY_RESULTS_COUNT] = NULL;

	DeeModelIter * iter = dee_model_append_row(query->results_model,
	                                                  columns /* variants */);

	dee_model_set_tag(query->results_model, iter, query->results_tag, result);

	return;
}

static void
hud_query_refresh (HudQuery *query)
{
  guint64 start_time;

  hud_watchdog_ping(query->watchdog);
  start_time = g_get_monotonic_time ();

  dee_model_clear(query->results_model);
  query->results_list = g_sequence_new(NULL);
  query->max_usage = 0;

  /* Note that the results are kept sorted as they are collected using a GSequence */
  HudSource * search_source = query->current_source;
  if (search_source == NULL) {
    search_source = hud_application_list_get_focused_app(query->app_list);
  }

  /* Make sure we've used it before searching it */
  if (search_source != NULL && search_source != query->last_used_source) {
    if (query->last_used_source != NULL)
    {
      hud_source_unuse(query->last_used_source);
      g_clear_object(&query->last_used_source);
    }
	hud_source_use(search_source);
	query->last_used_source = g_object_ref(search_source);
  }

  if (search_source != NULL) {
    hud_source_search (search_source, query->token_list, results_list_populate, query);
  } else {
    g_debug("Current source was null. This should usually not happen outside tests in regular user use");
  }
  g_debug("Num results: %d", g_sequence_get_length(query->results_list));

  g_sequence_foreach(query->results_list, results_list_max_usage, &query->max_usage);
  g_debug("Max Usage: %d", query->max_usage);

  /* Now that we have the max usage we can sort */
  g_sequence_sort(query->results_list, compare_func, query);
  g_sequence_foreach(query->results_list, results_list_to_model, query);

  /* NOTE: Not freeing the items as the references are picked up by the DeeModel */
  g_sequence_free(query->results_list);
  query->results_list = NULL;

  /* Reset for new data */
  dee_model_clear(query->appstack_model);

  /* Get the list of all applications that have data that is relevant
     to the current query, but just the app info. */
  GHashTable * appstack_hash = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, appstack_item_free);
  hud_source_list_applications (query->all_sources, query->token_list, app_results_list_populate, appstack_hash);

  /* If we've selected a source, make sure it's in the list */
  appstack_hash_add_source(appstack_hash, query->current_source, HUD_SOURCE_ITEM_TYPE_BACKGROUND_APP);

  /* Always have the focused app in there too */
  HudSource * focused = hud_application_list_get_focused_app(query->app_list);
  appstack_hash_add_source(appstack_hash, focused, HUD_SOURCE_ITEM_TYPE_FOCUSED_APP);

  /* Always have the side stage app in there too */
  HudSource * side_stage = hud_application_list_get_side_stage_focused_app(query->app_list);
  appstack_hash_add_source(appstack_hash, side_stage, HUD_SOURCE_ITEM_TYPE_SIDESTAGE_APP);

  /* Now take the hash, having already deduplicated for us, and turn
     it into a shorted DeeModel */
  appstack_hash_to_model(appstack_hash, query->appstack_model);

  /* Thanks hash ol' friend */
  g_hash_table_unref(appstack_hash);

  g_debug ("query took %dus\n", (int) (g_get_monotonic_time () - start_time));

  g_object_set(G_OBJECT(query->skel), "current-query", query->search_string, NULL);
}

static gboolean
hud_query_dispatch_refresh (gpointer user_data)
{
  HudQuery *query = user_data;

  hud_query_refresh (query);

  g_signal_emit (query, hud_query_changed_signal, 0);

  query->refresh_id = 0;

  return G_SOURCE_REMOVE;
}
static void
hud_query_source_changed (HudSource *source,
                          gpointer   user_data)
{
  HudQuery *query = user_data;

  if (!query->refresh_id)
    query->refresh_id = g_idle_add (hud_query_dispatch_refresh, query);
}

static void
hud_query_finalize (GObject *object)
{
  HudQuery *query = HUD_QUERY (object);

  g_debug ("Destroyed query '%s'", query->search_string);

  /* TODO: move to destroy */
  if (query->last_used_source != NULL)
  {
    hud_source_unuse(query->last_used_source);
    g_clear_object(&query->last_used_source);
  }

  g_clear_object(&query->skel);
  g_clear_object(&query->results_model);
  /* NOTE: ^^ Kills results_tag as well */
  g_clear_object(&query->appstack_model);

  if (query->refresh_id)
    g_source_remove (query->refresh_id);

  g_object_unref (query->all_sources);
  g_object_unref (query->app_list);
  g_clear_object (&query->current_source);
  if (query->token_list != NULL) {
    hud_token_list_free (query->token_list);
    query->token_list = NULL;
  }
  g_clear_pointer(&query->search_string, g_free);

  g_clear_pointer(&query->object_path, g_free);
  g_clear_pointer(&query->results_name, g_free);
  g_clear_pointer(&query->appstack_name, g_free);

  g_clear_object(&query->voice);
  g_clear_object(&query->watchdog);

  G_OBJECT_CLASS (hud_query_parent_class)
    ->finalize (object);
}

/* Handle the DBus function UpdateQuery */
static gboolean
handle_voice_query (HudQueryIfaceComCanonicalHudQuery * skel, GDBusMethodInvocation * invocation, gpointer user_data)
{
  g_return_val_if_fail(HUD_IS_QUERY(user_data), FALSE);
  HudQuery * query = HUD_QUERY(user_data);
  hud_watchdog_ping(query->watchdog);

  g_debug("Voice query is loading");
  hud_query_iface_com_canonical_hud_query_emit_voice_query_loading (
      HUD_QUERY_IFACE_COM_CANONICAL_HUD_QUERY (skel));
  gchar *voice_result;
  GError *error = NULL;

  HudSource * search_source = query->current_source;
  if (search_source == NULL) {
    search_source = hud_application_list_get_focused_app(query->app_list);
  }

  if (!hud_voice_query (query->voice, search_source, &voice_result, &error))
  {
    g_dbus_method_invocation_return_error_literal(invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED, error->message);
    g_error_free(error);
    return FALSE;
  }

  g_debug("Voice query is finished");

  if (voice_result == NULL)
    voice_result = g_strdup("");

  gchar *search_string = g_utf8_strdown(voice_result, -1);
  g_free(voice_result);

  g_debug("Updating Query to: '%s'", search_string);

  /* Clear the last query */
  g_clear_pointer(&query->search_string, g_free);
  if (query->token_list != NULL) {
    hud_token_list_free (query->token_list);
    query->token_list = NULL;
  }

  query->search_string = search_string;

  if (query->search_string[0] != '\0') {
    query->token_list = hud_token_list_new_from_string (query->search_string);
  }

  /* Refresh it all */
  hud_query_refresh (query);

  /* Tell DBus everything is going to be A-OK */
  hud_query_iface_com_canonical_hud_query_complete_voice_query(skel, invocation, 0, search_string);

  return TRUE;
}

/* Handle the DBus function UpdateQuery */
static gboolean
handle_update_query (HudQueryIfaceComCanonicalHudQuery * skel, GDBusMethodInvocation * invocation, const gchar * search_string, gpointer user_data)
{
	g_return_val_if_fail(HUD_IS_QUERY(user_data), FALSE);
	HudQuery * query = HUD_QUERY(user_data);
	hud_watchdog_ping(query->watchdog);

	g_debug("Updating Query to: '%s'", search_string);

	/* Clear the last query */
	g_clear_pointer(&query->search_string, g_free);
	if (query->token_list != NULL) {
		hud_token_list_free (query->token_list);
		query->token_list = NULL;
	}

	/* Grab the data from this one */
	query->search_string = g_strdup (search_string);
	query->search_string = g_strstrip(query->search_string);

	if (query->search_string[0] != '\0') {
		query->token_list = hud_token_list_new_from_string (query->search_string);
	}

	/* Refresh it all */
	hud_query_refresh (query);

	/* Tell DBus everything is going to be A-OK */
	hud_query_iface_com_canonical_hud_query_complete_update_query(skel, invocation, 0);

	return TRUE;
}

/* Handle the DBus function UpdateApp */
static gboolean
handle_update_app (HudQueryIfaceComCanonicalHudQuery * skel, GDBusMethodInvocation * invocation, const gchar * app_id, gpointer user_data)
{
	g_return_val_if_fail(HUD_IS_QUERY(user_data), FALSE);
	HudQuery * query = HUD_QUERY(user_data);
	hud_watchdog_ping(query->watchdog);

	g_debug("Updating App to: '%s'", app_id);

	g_clear_object (&query->current_source);
	query->current_source = hud_source_get(query->all_sources, app_id);
	if (query->current_source != NULL) {
		g_object_ref (query->current_source);
	}

	/* Refresh it all */
	hud_query_refresh (query);	

	/* Tell DBus everything is going to be A-OK */
	hud_query_iface_com_canonical_hud_query_complete_update_app(skel, invocation, 0);

	return TRUE;
}

/* Handle getting execute from DBus */
static gboolean
handle_execute (HudQueryIfaceComCanonicalHudQuery * skel, GDBusMethodInvocation * invocation, GVariant * command_id, guint timestamp, gpointer user_data)
{
	g_return_val_if_fail(HUD_IS_QUERY(user_data), FALSE);
	HudQuery * query = HUD_QUERY(user_data);
	hud_watchdog_ping(query->watchdog);

	/* Do good */
	GVariant * inner = g_variant_get_variant(command_id);
	guint64 id = g_variant_get_uint64(inner);
	g_variant_unref(inner);

	HudItem * item = hud_item_lookup(id);

	if (item == NULL) {
		g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS, "Item specified by command key does not exist");
		return TRUE;
	}

	GVariantBuilder platform;
	g_variant_builder_init(&platform, G_VARIANT_TYPE_DICTIONARY);

	GVariantBuilder entry;
	g_variant_builder_init(&entry, G_VARIANT_TYPE_DICT_ENTRY);
	g_variant_builder_add_value(&entry, g_variant_new_string("desktop-startup-id"));
	gchar * timestr = g_strdup_printf("_TIME%d", timestamp);
	g_variant_builder_add_value(&entry, g_variant_new_variant(g_variant_new_string(timestr)));
	g_free(timestr);

	g_variant_builder_add_value(&platform, g_variant_builder_end(&entry));

	hud_item_activate(item, g_variant_builder_end(&platform));

	hud_query_iface_com_canonical_hud_query_complete_execute_command(skel, invocation);

	return TRUE;
}

/* Handle getting parameterized from DBus */
static gboolean
handle_parameterized (HudQueryIfaceComCanonicalHudQuery * skel, GDBusMethodInvocation * invocation, GVariant * command_id, guint timestamp, gpointer user_data)
{
	g_return_val_if_fail(HUD_IS_QUERY(user_data), FALSE);
	HudQuery * query = HUD_QUERY(user_data);
	hud_watchdog_ping(query->watchdog);

	GVariant * inner = g_variant_get_variant(command_id);
	guint64 id = g_variant_get_uint64(inner);
	g_variant_unref(inner);

	HudItem * item = hud_item_lookup(id);

	if (item == NULL) {
		g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS, "Item specified by command key does not exist");
		return TRUE;
	}

	if (!HUD_IS_MODEL_ITEM(item)) {
		g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS, "Item specified by command is not a menu model item");
		return TRUE;
	}

	if (!hud_model_item_is_parameterized(HUD_MODEL_ITEM(item))) {
		g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS, "Item specified by command does not have parameterized actions");
		return TRUE;
	}

	const gchar * base_action = NULL;
	const gchar * action_path = NULL;
	const gchar * model_path = NULL;
	gint model_section = 0;

	hud_model_item_activate_parameterized(HUD_MODEL_ITEM(item), timestamp, &base_action, &action_path, &model_path, &model_section);

	if (base_action == NULL) {
		/* This value can be NULL, but variants require an empty string */
		base_action = "";
	}

	if (base_action == NULL ||
			action_path == NULL || !g_variant_is_object_path(action_path) ||
			model_path == NULL || !g_variant_is_object_path(model_path)) {
		g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED, "Values returned by the model item are invalid");
		return TRUE;
	}

  hud_query_iface_com_canonical_hud_query_complete_execute_parameterized (skel,
      invocation, base_action, action_path, model_path, model_section);
	return TRUE;
}

static gboolean
handle_execute_toolbar (HudQueryIfaceComCanonicalHudQuery *object, GDBusMethodInvocation *invocation, const gchar *arg_item, guint arg_timestamp, gpointer user_data)
{
	g_return_val_if_fail(HUD_IS_QUERY(user_data), FALSE);
	HudQuery * query = HUD_QUERY(user_data);
	hud_watchdog_ping(query->watchdog);

	HudClientQueryToolbarItems item = hud_client_query_toolbar_items_get_value_from_nick(arg_item);

	if (item != HUD_CLIENT_QUERY_TOOLBAR_QUIT) {
		g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED, "TODO");
		return TRUE;
	}

	HudSource * search_source = query->current_source;
	if (search_source == NULL) {
		search_source = hud_application_list_get_focused_app(query->app_list);
	}

	if (search_source == NULL) {
		g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED, "No source currently in use");
		return TRUE;
	}

	GVariantBuilder platform;
	g_variant_builder_init(&platform, G_VARIANT_TYPE_DICTIONARY);

	GVariantBuilder entry;
	g_variant_builder_init(&entry, G_VARIANT_TYPE_DICT_ENTRY);
	g_variant_builder_add_value(&entry, g_variant_new_string("desktop-startup-id"));
	gchar * timestr = g_strdup_printf("_TIME%d", arg_timestamp);
	g_variant_builder_add_value(&entry, g_variant_new_variant(g_variant_new_string(timestr)));
	g_free(timestr);

	g_variant_builder_add_value(&platform, g_variant_builder_end(&entry));

	hud_source_activate_toolbar(search_source, item, g_variant_builder_end(&platform));
	g_dbus_method_invocation_return_value(invocation, NULL);
	return TRUE;
}

/* Really this is just an unref, but let's put it in a nice function
   so that it's easier to change later if we need to. */
void
hud_query_close (HudQuery * query)
{
	g_return_if_fail(HUD_IS_QUERY(query));
	g_object_unref(query);
	return;
}

/* Handle the DBus function CloseQuery */
static gboolean
handle_close_query (HudQueryIfaceComCanonicalHudQuery * skel, GDBusMethodInvocation * invocation, gpointer user_data)
{
	g_return_val_if_fail(HUD_IS_QUERY(user_data), FALSE);
	HudQuery * query = HUD_QUERY(user_data);
	hud_watchdog_ping(query->watchdog);

	/* Close the query */
	hud_query_close(query);

	/* NOTE: Don't use the query after this, it may not exist */
	query = NULL;

	/* Tell DBus we're dying */
	hud_query_iface_com_canonical_hud_query_complete_close_query(skel, invocation);

	return TRUE;
}

static void
hud_query_init_real (HudQuery *query, GDBusConnection *connection, const guint querynumber)
{
  GError *error = NULL;

  query->querynumber = querynumber;

  query->skel = hud_query_iface_com_canonical_hud_query_skeleton_new();

  /* NOTE: Connect to the functions before putting on the bus. */
  g_signal_connect(G_OBJECT(query->skel), "handle-update-query", G_CALLBACK(handle_update_query), query);
  g_signal_connect(G_OBJECT(query->skel), "handle-update-app", G_CALLBACK(handle_update_app), query);
  g_signal_connect(G_OBJECT(query->skel), "handle-voice-query", G_CALLBACK(handle_voice_query), query);
  g_signal_connect(G_OBJECT(query->skel), "handle-close-query", G_CALLBACK(handle_close_query), query);
  g_signal_connect(G_OBJECT(query->skel), "handle-execute-command", G_CALLBACK(handle_execute), query);
  g_signal_connect(G_OBJECT(query->skel), "handle-execute-parameterized", G_CALLBACK(handle_parameterized), query);
  g_signal_connect(G_OBJECT(query->skel), "handle-execute-toolbar", G_CALLBACK(handle_execute_toolbar), query);

  query->object_path = g_strdup_printf("/com/canonical/hud/query%d", query->querynumber);
  if (!g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(query->skel),
                                   connection,
                                   query->object_path,
                                   &error))
  {
    g_warning ("%s %s\n", "g_dbus_interface_skeleton_export failed:", error->message);
    g_error_free(error);
  }

  GDBusInterfaceInfo* info = g_dbus_interface_skeleton_get_info (
      G_DBUS_INTERFACE_SKELETON(query->skel) );
  g_debug("Created interface skeleton: [%s] on [%s]", info->name, g_dbus_interface_skeleton_get_object_path(G_DBUS_INTERFACE_SKELETON(query->skel)));

  query->results_name = g_strdup_printf("com.canonical.hud.query%d.results", query->querynumber);
  query->results_model = dee_shared_model_new(query->results_name);
  dee_model_set_schema_full(query->results_model, results_model_schema, G_N_ELEMENTS(results_model_schema));
  query->results_tag = dee_model_register_tag(query->results_model, g_object_unref);

  query->appstack_name = g_strdup_printf("com.canonical.hud.query%d.appstack", query->querynumber);
  query->appstack_model = dee_shared_model_new(query->appstack_name);
  dee_model_set_schema_full(query->appstack_model, appstack_model_schema, G_N_ELEMENTS(appstack_model_schema));

  g_object_set(G_OBJECT(query->skel),
               "appstack-model", query->appstack_name,
               "results-model", query->results_name,
               NULL);

  g_dbus_interface_skeleton_flush(G_DBUS_INTERFACE_SKELETON(query->skel));

  error = NULL;
  query->voice = hud_voice_new(query->skel, NULL, &error);
  if (!query->voice)
  {
    g_warning ("%s %s\n", "Voice engine failed to initialize:", error->message);
    g_error_free(error);
  }
}

static void
hud_query_init (HudQuery *query)
{
}

static void
hud_query_class_init (HudQueryClass *class)
{
  /**
   * HudQuery::changed:
   * @query: a #HudQuery
   *
   * Indicates that the results of @query have changed.
   **/
  hud_query_changed_signal = g_signal_new ("changed", HUD_TYPE_QUERY, G_SIGNAL_RUN_LAST, 0, NULL,
                                           NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  class->finalize = hud_query_finalize;
}

/**
 * hud_query_new:
 * @source: the #HudSource against which to search
 * @search_string: the string to search for
 * @num_results: the maximum number of results to report
 *
 * Creates a #HudQuery.
 *
 * A #HudQuery is a stateful search for @search_string against a @source.
 *
 * Each #HudQuery is assigned a "query key" when it is created.  This
 * can be used to lookup the hud query later using hud_query_lookup().
 * Because of this, an internal reference is held on the query and the
 * query won't be completely freed until you call hud_query_close() on
 * it in addition to releasing your ref.
 *
 * Returns: the new #HudQuery
 **/
HudQuery *
hud_query_new (HudSource   *all_sources,
               HudWatchdog *watchdog,
               HudApplicationList *application_list,
               const gchar *search_string,
               gint         num_results,
               GDBusConnection *connection,
               const guint  query_count)
{
  HudQuery *query;

  g_debug ("Created query '%s'", search_string);

  query = g_object_new (HUD_TYPE_QUERY, NULL);
  hud_query_init_real(query, connection, query_count);
  query->all_sources = g_object_ref (all_sources);
  query->app_list = g_object_ref (application_list);
  query->search_string = g_strdup (search_string);
  query->token_list = NULL;

  if (watchdog != NULL) {
    query->watchdog = g_object_ref(watchdog);
  }
  
  if (query->search_string[0] != '\0') {
    query->token_list = hud_token_list_new_from_string (query->search_string);
  }

  query->num_results = num_results;

  hud_query_refresh (query);

  g_signal_connect_object (all_sources, "changed", G_CALLBACK (hud_query_source_changed), query, 0);

  return query;
}

/**
 * hud_query_get_path:
 * @query: a #HudQuery
 *
 * Gets the path that the query object is exported to DBus on.
 *
 * Return value: A dbus object path
 */
const gchar *
hud_query_get_path (HudQuery    *query)
{
	g_return_val_if_fail(HUD_IS_QUERY(query), NULL);

	return query->object_path;
}

/**
 * hud_query_get_results_name:
 * @query: a #HudQuery
 *
 * Gets the DBus name that the shared results model is using
 *
 * Return value: A dbus name
 */
const gchar *
hud_query_get_results_name (HudQuery    *query)
{
	g_return_val_if_fail(HUD_IS_QUERY(query), NULL);

	return query->results_name;
}

/**
 * hud_query_get_appstack_name:
 * @query: a #HudQuery
 *
 * Gets the DBus name that the appstack model is using
 *
 * Return value: A dbus name
 */
const gchar *
hud_query_get_appstack_name (HudQuery * query)
{
	g_return_val_if_fail(HUD_IS_QUERY(query), NULL);

	return query->appstack_name;
}

DeeModel *
hud_query_get_results_model(HudQuery *self)
{
  g_return_val_if_fail(HUD_IS_QUERY(self), NULL);

  return self->results_model;

}

DeeModel *
hud_query_get_appstack_model(HudQuery *self)
{
  g_return_val_if_fail(HUD_IS_QUERY(self), NULL);

  return self->appstack_model;

}

const gchar *
hud_query_get_query (HudQuery * query)
{
	g_return_val_if_fail(HUD_IS_QUERY(query), NULL);

	return query->search_string;
}

guint
hud_query_get_number (HudQuery * query)
{
	g_return_val_if_fail(HUD_IS_QUERY(query), 0);

	return query->querynumber;
}
