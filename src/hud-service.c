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

#define G_LOG_DOMAIN "hud-service"

#include <glib.h>
#include <gio/gio.h>
#include <glib/gi18n.h>
#include <stdlib.h>
#include <locale.h>
#include <libintl.h>

#include "hudappindicatorsource.h"
#include "hudindicatorsource.h"
#include "hudwebappsource.h"
#include "huddebugsource.h"
#include "hudsourcelist.h"
#include "hudsettings.h"
#include "application-list.h"
#include "query-columns.h"

#include "hud-iface.h"
#include "shared-values.h"
#include "hudquery.h"
#include "watchdog.h"

/* Prototypes */
static void            bus_method         (GDBusConnection       *connection,
                                           const gchar           *sender,
                                           const gchar           *object_path,
                                           const gchar           *interface_name,
                                           const gchar           *method_name,
                                           GVariant              *parameters,
                                           GDBusMethodInvocation *invocation,
                                           gpointer               user_data);
static GVariant *      bus_get_prop       (GDBusConnection *      connection,
                                           const gchar *          sender,
                                           const gchar *          object_path,
                                           const gchar *          interface_name,
                                           const gchar *          property_name,
                                           GError **              error,
                                           gpointer               user_data);


/* Globals */
static guint query_count = 0;
static GPtrArray * query_list = NULL;
static GMainLoop *mainloop = NULL;
static GDBusInterfaceVTable vtable = {
	.method_call = bus_method,
	.get_property = bus_get_prop,
	.set_property = NULL
};
static HudApplicationList * application_list = NULL;
static HudWatchdog * watchdog = NULL;

/* Get our error domain */
GQuark
error (void)
{
	static GQuark err = 0;
	if (err == 0) {
		err = g_quark_from_static_string("hud");
	}
	return err;
}

/* Describe the important values in the query */
GVariant *
describe_query (HudQuery *query)
{
  GVariantBuilder builder;

  g_variant_builder_init (&builder, G_VARIANT_TYPE_TUPLE);

  g_variant_builder_add_value(&builder, g_variant_new_object_path(hud_query_get_path(query)));
  g_variant_builder_add_value(&builder, g_variant_new_string(hud_query_get_results_name(query)));
  g_variant_builder_add_value(&builder, g_variant_new_string(hud_query_get_appstack_name(query)));
  g_variant_builder_add_value(&builder, g_variant_new_int32(0)); /* TODO: Get model rev */

  return g_variant_builder_end (&builder);
}

/* Builds a single line pango formated description for
   the legacy HUD UI */
static gchar *
build_legacy_description (DeeModel * model, DeeModelIter * iter)
{
	const gchar * command_name = dee_model_get_string(model, iter, HUD_QUERY_RESULTS_COMMAND_NAME);
	const gchar * description = dee_model_get_string(model, iter, HUD_QUERY_RESULTS_DESCRIPTION);

	gchar * combined;
	if (description != NULL && strlen(description) > 1) {
		/* TRANSLATORS: This is what is shown for Unity Nux in
		   the HUD entries.  The first %s is the command name and
		   the second is a description or list of keywords that
		   was used to find the entry. */
		combined = g_strdup_printf(_("%s\xE2\x80\x82(%s)"), command_name, description);
	} else {
		combined = g_strdup(command_name);
	}

	gchar * retval = g_markup_escape_text(combined, -1);
	g_free(combined);

	/* TODO: Highlights */

	return retval;
}

/* Describe the legacy query */
GVariant *
describe_legacy_query (HudQuery * query)
{
	GVariantBuilder builder;
	g_variant_builder_init(&builder, G_VARIANT_TYPE_TUPLE);

	g_variant_builder_add_value(&builder, g_variant_new_string(hud_query_get_query(query)));

	gboolean item_added = FALSE;
	DeeModel * results = hud_query_get_results_model(query);
	if (dee_model_get_n_rows(results) > 0) {
		/* Get the application icon from the appstack */
		DeeModel * appstack = hud_query_get_appstack_model(query);
		GVariant * app_icon = NULL;

		if (appstack != NULL && dee_model_get_n_rows(appstack) > 0) {
			app_icon = dee_model_get_value(appstack, dee_model_get_first_iter(appstack), 1);
		}

		if (app_icon == NULL) {
			app_icon = g_variant_new_string("");
		}

		/* Setup loop */
		DeeModelIter * iter = dee_model_get_first_iter(results);
		int i;

		/* Parse through either the first five results or the full list */
		for (i = 0; i < 5 && iter != NULL; i++, iter = dee_model_next(results, iter)) {
			/* Don't show parameterized actions */
			if (dee_model_get_bool(results, iter, 7)) {
				i--;
				continue;
			}

			if (!item_added) {
				/* Open the builder to put in the array */
				g_variant_builder_open(&builder, G_VARIANT_TYPE_ARRAY);
				item_added = TRUE;
			}

			g_variant_builder_open(&builder, G_VARIANT_TYPE_TUPLE);

			/* Description */
			gchar * desc = build_legacy_description(results, iter);
			g_variant_builder_add_value(&builder, g_variant_new_string(desc));
			g_free(desc);

			/* Icon */
			g_variant_builder_add_value(&builder, app_icon);

			/* 3 Blanks */
			GVariant * blank = g_variant_new_string("");
			g_variant_builder_add_value(&builder, blank);
			g_variant_builder_add_value(&builder, blank);
			g_variant_builder_add_value(&builder, blank);

			/* ID */
			g_variant_builder_add_value(&builder, dee_model_get_value(results, iter, 0));

			g_variant_builder_close(&builder);
		}

		if (item_added) {
			g_variant_builder_close(&builder);
		}
	} else {
		g_debug("Dee Results Model is empty");
	}
	
	if (!item_added) {
		g_variant_builder_add_value(&builder, g_variant_new_array(G_VARIANT_TYPE("(sssssv)"), NULL, 0));
	}

	g_variant_builder_add_value(&builder, g_variant_new_variant(g_variant_new_uint32(hud_query_get_number(query))));

	return g_variant_builder_end(&builder);
}

/* Respond to the query being updated and send a signal on
   DBus for it */
static void
legacy_update (HudQuery * query, gpointer user_data)
{
	GDBusConnection * connection = G_DBUS_CONNECTION(user_data);
	GError * error = NULL;

	g_dbus_connection_emit_signal(connection,
		NULL, /* destination */
		"/com/canonical/hud",
		"com.canonical.hud",
		"UpdatedQuery",
		describe_legacy_query(query),
		&error);

	if (error != NULL) {
		g_warning("Unable to signal a query update: %s", error->message);
		g_error_free(error);
	}

	return;
}

/* Respond to the query being destroyed by removing it from
   the list */
static void
query_destroyed (gpointer data, GObject * old_object)
{
	GPtrArray * list = (GPtrArray *)data;
	g_ptr_array_remove(list, old_object);
	return;
}

/* Build a query and put it into the query list */
static HudQuery *
build_query (HudSourceList * all_sources, HudApplicationList * app_list, GDBusConnection * connection, const gchar * search_string)
{
	HudQuery * query = hud_query_new (HUD_SOURCE(all_sources), watchdog, application_list, search_string, 10, connection, ++query_count);

	g_ptr_array_add(query_list, query);
	g_object_weak_ref(G_OBJECT(query), query_destroyed, query_list);

	return query;
}

/* Make the time platform data thingy */
static GVariant *
unpack_platform_data (GVariant *parameters)
{
  GVariant *platform_data;
  gchar *startup_id;
  guint32 timestamp;

  g_variant_get_child (parameters, 1, "u", &timestamp);
  startup_id = g_strdup_printf ("_TIME%u", timestamp);
  platform_data = g_variant_new_parsed ("{'desktop-startup-id': < %s >}", startup_id);
  g_free (startup_id);

  return g_variant_ref_sink (platform_data);
}

/* Take a method call from DBus */
static void
bus_method (GDBusConnection       *connection,
            const gchar           *sender,
            const gchar           *object_path,
            const gchar           *interface_name,
            const gchar           *method_name,
            GVariant              *parameters,
            GDBusMethodInvocation *invocation,
            gpointer               user_data)
{
	hud_watchdog_ping(watchdog);

	if (g_str_equal (method_name, "CreateQuery")) {
		HudSourceList *all_sources = user_data;
		GVariant * vsearch;
		const gchar *search_string;
		HudQuery *query;

		vsearch = g_variant_get_child_value (parameters, 0);
		search_string = g_variant_get_string(vsearch, NULL);
		g_debug ("'CreateQuery' from %s: '%s'", sender, search_string);

		query = build_query (all_sources, application_list, connection, search_string);
		g_dbus_method_invocation_return_value (invocation, describe_query (query));

		g_variant_unref(vsearch);
	} else if (g_str_equal (method_name, "StartQuery")) {
		HudSourceList *all_sources = user_data;
		GVariant * vsearch;
		const gchar *search_string;
		HudQuery *query;

		/* Legacy inteface for Compiz-based Unity */
		vsearch = g_variant_get_child_value (parameters, 0);
		search_string = g_variant_get_string(vsearch, NULL);
		g_debug ("'StartQuery' from %s: '%s'", sender, search_string);

		query = build_query (all_sources, application_list, connection, search_string);
		g_signal_connect(query, "changed", G_CALLBACK(legacy_update), connection);
		g_dbus_method_invocation_return_value (invocation, describe_legacy_query (query));

		g_variant_unref(vsearch);
	} else if (g_str_equal (method_name, "CloseQuery")) {
		/* Legacy interface to close a query */
		GVariant * vvvquery = g_variant_get_child_value (parameters, 0);
		GVariant * vvquery = g_variant_get_variant(vvvquery);
		GVariant * vquery = g_variant_get_variant(vvquery);
		guint query_number = g_variant_get_uint32(vquery);
		g_variant_unref(vquery);
		g_variant_unref(vvquery);
		g_variant_unref(vvvquery);

		/* Find the query */
		int i;
		HudQuery * query = NULL;
		for (i = 0; i < query_list->len; i++) {
			if (hud_query_get_number(g_ptr_array_index(query_list, i)) == query_number) {
				query = g_ptr_array_index(query_list, i);
				break;
			}
		}

		if (query != NULL) {
			hud_query_close(query);
			g_dbus_method_invocation_return_value (invocation, NULL);
		} else {
			g_dbus_method_invocation_return_error_literal(invocation, error(), 2, "Unable to find Query");
		}
	} else if (g_str_equal (method_name, "ExecuteQuery")) {
		/* Legacy interface to execute a query */
		GVariant *platform_data;
		GVariant *item_key;
		guint64 key_value;
		HudItem *item;

		g_variant_get_child (parameters, 0, "v", &item_key);

		if (!g_variant_is_of_type (item_key, G_VARIANT_TYPE_UINT64)) {
			g_debug ("'ExecuteQuery' from %s: incorrect item key (not uint64)", sender);
			g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
			                                       "item key has invalid format");
			g_variant_unref (item_key);
			return;
		}

		key_value = g_variant_get_uint64 (item_key);
		g_variant_unref (item_key);

		item = hud_item_lookup (key_value);
		g_debug ("'ExecuteQuery' from %s, item #%"G_GUINT64_FORMAT": %p", sender, key_value, item);

		if (item == NULL) {
			g_dbus_method_invocation_return_error (invocation, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
			                                       "item specified by item key does not exist");
			return;
		}

		platform_data = unpack_platform_data (parameters);
		hud_item_activate (item, platform_data);
		g_variant_unref (platform_data);

		g_dbus_method_invocation_return_value (invocation, NULL);
	} else if (g_str_equal (method_name, "RegisterApplication")) {
		GVariant * vid = g_variant_get_child_value (parameters, 0);

		HudApplicationSource * appsource = hud_application_list_get_source(application_list, g_variant_get_string(vid, NULL));

		const gchar * path = NULL;
		if (appsource != NULL) {
			path = hud_application_source_get_path(appsource);
		}

		GVariant * vpath = NULL;
		if (path != NULL) {
			vpath = g_variant_new_object_path(path);
		}

		if (vpath != NULL) {
			g_dbus_method_invocation_return_value(invocation, g_variant_new_tuple(&vpath, 1));
		} else {
			g_dbus_method_invocation_return_error_literal(invocation, error(), 1, "Unable to get path for the created application");
		}
		g_variant_unref(vid);
	} else {
		g_warn_if_reached();
	}

	return;
}

/* Gets properties for DBus */
static GVariant *
bus_get_prop (GDBusConnection * connection, const gchar * sender, const gchar * object_path, const gchar * interface_name, const gchar * property_name, GError ** error, gpointer user_data)
{
	hud_watchdog_ping(watchdog);
	// HudSource *source = user_data;

	if (g_str_equal(property_name, "OpenQueries")) {
		if (query_list->len == 0) {
			return g_variant_new_array(G_VARIANT_TYPE_OBJECT_PATH, NULL, 0);
		}

		int i;
		GVariantBuilder builder;
		g_variant_builder_init(&builder, G_VARIANT_TYPE_ARRAY);

		for (i = 0; i < query_list->len; i++) {
			HudQuery * query = g_ptr_array_index(query_list, i);
			g_variant_builder_add_value(&builder, g_variant_new_object_path(hud_query_get_path(query)));
		}

		return g_variant_builder_end(&builder);
	} else if (g_str_equal(property_name, "Applications")) {
		GList * apps = hud_application_list_get_apps(application_list);
		if (apps == NULL) {
			return g_variant_new_array(G_VARIANT_TYPE_OBJECT_PATH, NULL, 0);
		}

		GVariantBuilder builder;
		g_variant_builder_init(&builder, G_VARIANT_TYPE_ARRAY);
		while (apps != NULL) {
			HudApplicationSource * source = HUD_APPLICATION_SOURCE(apps->data);
			g_variant_builder_open(&builder, G_VARIANT_TYPE_TUPLE);
			g_variant_builder_add_value(&builder, g_variant_new_string(hud_application_source_get_id(source)));
			g_variant_builder_add_value(&builder, g_variant_new_object_path(hud_application_source_get_path(source)));
			g_variant_builder_close(&builder);
			apps = g_list_next(apps);
		}

		return g_variant_builder_end(&builder);
	} else {
		g_warn_if_reached();
	}

	return NULL;
}

static void
bus_acquired_cb (GDBusConnection *connection,
                 const gchar     *name,
                 gpointer         user_data)
{
  HudSourceList *source_list = user_data;
  GError *error = NULL;

  g_debug ("Bus acquired (guid %s)", g_dbus_connection_get_guid (connection));

  {
    HudIndicatorSource *source;

    source = hud_indicator_source_new (connection);
    hud_source_list_add (source_list, HUD_SOURCE (source));
    g_object_unref (source);
  }

  {
    HudAppIndicatorSource *source;

    source = hud_app_indicator_source_new (connection);
    hud_source_list_add (source_list, HUD_SOURCE (source));
    g_object_unref (source);
  }

  {
    HudWebappSource *source;

    source = hud_webapp_source_new ();
    hud_source_list_add (source_list, HUD_SOURCE (source));

    g_object_unref (G_OBJECT (source));
  }

  if (!g_dbus_connection_register_object (connection, DBUS_PATH, hud_iface_com_canonical_hud_interface_info (), &vtable, source_list, NULL, &error))
    {
      g_warning ("Unable to register path '"DBUS_PATH"': %s", error->message);
      g_main_loop_quit (mainloop);
      g_error_free (error);
    }
}

static void
name_acquired_cb (GDBusConnection *connection,
                  const gchar     *name,
                  gpointer         user_data)
{
  g_debug ("Acquired bus name '%s'", name);
}

static void
name_lost_cb (GDBusConnection *connection,
              const gchar     *name,
              gpointer         user_data)
{
  g_warning ("Unable to get name '%s'", name);
  g_main_loop_quit (mainloop);
}

static void
sigterm_graceful_exit (int signal)
{
  g_warning("SIGTERM recieved");
  g_main_loop_quit(mainloop);
  return;
}

int
main (int argc, char **argv)
{
  HudSourceList *source_list;

#ifndef GLIB_VERSION_2_36
  g_type_init ();
#endif

  setlocale (LC_ALL, "");
  bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
  textdomain (GETTEXT_PACKAGE);

  hud_settings_init ();

  query_list = g_ptr_array_new();
  source_list = hud_source_list_new ();

  application_list = hud_application_list_new();
  hud_source_list_add(source_list, HUD_SOURCE(application_list));

  if (getenv ("HUD_DEBUG_SOURCE"))
    {
      HudDebugSource *source;

      source = hud_debug_source_new ();
      hud_source_list_add (source_list, HUD_SOURCE (source));
      g_object_unref (source);
    }

  g_bus_own_name (G_BUS_TYPE_SESSION, DBUS_NAME, G_BUS_NAME_OWNER_FLAGS_NONE,
                  bus_acquired_cb, name_acquired_cb, name_lost_cb, source_list, NULL);

  mainloop = g_main_loop_new (NULL, FALSE);

  signal(SIGTERM, sigterm_graceful_exit);
  
  watchdog = hud_watchdog_new(mainloop);

  g_main_loop_run (mainloop);
  g_main_loop_unref (mainloop);

  g_clear_object(&watchdog);

  g_object_unref (application_list);
  g_object_unref (source_list);
  g_ptr_array_free(query_list, TRUE);

  return 0;
}
