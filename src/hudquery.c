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

#include "hudquery.h"
#include "hud-query-iface.h"

#include "hudresult.h"

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

  HudSource *source;
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
};

typedef GObjectClass HudQueryClass;

G_DEFINE_TYPE (HudQuery, hud_query, G_TYPE_OBJECT)

static guint hud_query_changed_signal;

static guint query_count = 0;

static const gchar * results_model_schema[] = {
	"v", /* Command ID */
	"s", /* Command Name */
	"a(ii)", /* Highlights in command name */
	"s", /* Description */
	"a(ii)", /* Highlights in description */
	"s", /* Shortcut */
	"u", /* Distance */
};

static const gchar * appstack_model_schema[] = {
	"s", /* Application ID */
	"s", /* Icon Name */
};

static gint
compare_func (GVariant   ** a,
              GVariant   ** b,
              gpointer      user_data)
{
  guint distance_a;
  guint distance_b;

  distance_a = g_variant_get_uint32(a[6]);
  distance_b = g_variant_get_uint32(b[6]);

  return distance_a - distance_b;
}

/* Add a HudResult to the list of results */
static void
results_list_populate (HudResult * result, gpointer user_data)
{
	HudQuery * query = (HudQuery *)user_data;
	HudItem * item = hud_result_get_item(result);
	gchar * context = NULL; /* Need to free this one, sucks, practical reality */

	GVariant * columns[G_N_ELEMENTS(results_model_schema) + 1];
	columns[0] = g_variant_new_variant(g_variant_new_uint64(hud_item_get_id(item)));
	columns[1] = g_variant_new_string(hud_item_get_command(item));
	columns[2] = g_variant_new_array(G_VARIANT_TYPE("(ii)"), NULL, 0);
	columns[3] = g_variant_new_string(context = hud_item_get_context(item));
	columns[4] = g_variant_new_array(G_VARIANT_TYPE("(ii)"), NULL, 0);
	columns[5] = g_variant_new_string(hud_item_get_shortcut(item));
	columns[6] = g_variant_new_uint32(hud_result_get_distance(result, 0)); /* TODO: Figure out max usage */
	columns[7] = NULL;

	g_free(context);

	DeeModelIter * iter = dee_model_insert_row_sorted(query->results_model,
	                                                  columns /* variants */,
	                                                  compare_func, NULL);

	dee_model_set_tag(query->results_model, iter, query->results_tag, result);

	return;
}

static void
hud_query_refresh (HudQuery *query)
{
  guint64 start_time;

  start_time = g_get_monotonic_time ();

  dee_model_clear(query->results_model);

  if (hud_token_list_get_length (query->token_list) != 0)
    hud_source_search (query->source, query->token_list, results_list_populate, query);

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
  g_clear_object(&query->skel);
  g_clear_object(&query->results_model);
  /* NOTE: ^^ Kills results_tag as well */
  g_clear_object(&query->appstack_model);

  if (query->refresh_id)
    g_source_remove (query->refresh_id);

  hud_source_unuse (query->source);

  g_object_unref (query->source);
  hud_token_list_free (query->token_list);
  g_free (query->search_string);

  g_clear_pointer(&query->object_path, g_free);
  g_clear_pointer(&query->results_name, g_free);
  g_clear_pointer(&query->appstack_name, g_free);

  G_OBJECT_CLASS (hud_query_parent_class)
    ->finalize (object);
}

/* Handle the DBus function UpdateQuery */
static gboolean
handle_update_query (HudQueryIfaceComCanonicalHudQuery * skel, GDBusMethodInvocation * invocation, const gchar * search_string, gpointer user_data)
{
	g_return_val_if_fail(HUD_IS_QUERY(user_data), FALSE);
	HudQuery * query = HUD_QUERY(user_data);

	g_debug("Updating Query to: '%s'", search_string);

	/* Clear the last query */
	g_clear_pointer(&query->search_string, g_free);
	hud_token_list_free (query->token_list);
	query->token_list = NULL;

	/* Grab the data from this one */
	query->search_string = g_strdup (search_string);
	query->token_list = hud_token_list_new_from_string (query->search_string);

	/* Refresh it all */
	hud_query_refresh (query);

	/* Tell DBus everything is going to be A-OK */
	GVariant * modelrev = g_variant_new_int32(0);
	g_dbus_method_invocation_return_value(invocation, g_variant_new_tuple(&modelrev, 1));

	return TRUE;
}

/* Handle getting execute from DBus */
static gboolean
handle_execute (HudQueryIfaceComCanonicalHudQuery * skel, GDBusMethodInvocation * invocation, GVariant * command_id, guint timestamp, gpointer user_data)
{
	g_return_val_if_fail(HUD_IS_QUERY(user_data), FALSE);
	//HudQuery * query = HUD_QUERY(user_data);

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

	g_dbus_method_invocation_return_value(invocation, NULL);

	return TRUE;
}

/* Handle the DBus function UpdateQuery */
static gboolean
handle_close_query (HudQueryIfaceComCanonicalHudQuery * skel, GDBusMethodInvocation * invocation, gpointer user_data)
{
	g_return_val_if_fail(HUD_IS_QUERY(user_data), FALSE);
	HudQuery * query = HUD_QUERY(user_data);

	/* Unref the query */
	g_object_unref(query);

	/* NOTE: Don't use the query after this, it may not exist */
	query = NULL;

	/* Tell DBus we're dying */
	g_dbus_method_invocation_return_value(invocation, NULL);

	return TRUE;
}

static void
hud_query_init (HudQuery *query)
{
  query->querynumber = query_count++;

  query->skel = hud_query_iface_com_canonical_hud_query_skeleton_new();

  /* NOTE: Connect to the functions before putting on the bus. */
  g_signal_connect(G_OBJECT(query->skel), "handle-update-query", G_CALLBACK(handle_update_query), query);
  g_signal_connect(G_OBJECT(query->skel), "handle-close-query", G_CALLBACK(handle_close_query), query);
  g_signal_connect(G_OBJECT(query->skel), "handle-execute-command", G_CALLBACK(handle_execute), query);

  query->object_path = g_strdup_printf("/com/canonical/hud/query%d", query->querynumber);
  g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(query->skel),
                                   g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, NULL),
                                   query->object_path,
                                   NULL);

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

  return;
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
hud_query_new (HudSource   *source,
               const gchar *search_string,
               gint         num_results)
{
  HudQuery *query;

  g_debug ("Created query '%s'", search_string);

  query = g_object_new (HUD_TYPE_QUERY, NULL);
  query->source = g_object_ref (source);
  query->search_string = g_strdup (search_string);
  query->token_list = hud_token_list_new_from_string (query->search_string);
  query->num_results = num_results;

  hud_source_use (query->source);

  hud_query_refresh (query);

  g_signal_connect_object (source, "changed", G_CALLBACK (hud_query_source_changed), query, 0);

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
