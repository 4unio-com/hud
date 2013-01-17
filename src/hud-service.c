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

#include "config.h"

#include <glib.h>
#include <gio/gio.h>
#include <stdlib.h>
#include <locale.h>
#include <libintl.h>

#include "hudappindicatorsource.h"
#include "hudindicatorsource.h"
#include "hudwebappsource.h"
#include "hudwindowsource.h"
#include "huddebugsource.h"
#include "hudsourcelist.h"
#include "hudsettings.h"

#include "hud-iface.h"
#include "shared-values.h"
#include "hudquery.h"

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

static void
query_destroyed (gpointer data, GObject * old_object)
{
	GPtrArray * list = (GPtrArray *)data;
	g_ptr_array_remove(list, old_object);
	return;
}

static GPtrArray * query_list = NULL;

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
  HudSource *source = user_data;

  if (g_str_equal (method_name, "StartQuery"))
    {
      GVariant * vsearch;
      const gchar *search_string;
      HudQuery *query;

      vsearch = g_variant_get_child_value (parameters, 0);
      search_string = g_variant_get_string(vsearch, NULL);
      g_debug ("'StartQuery' from %s: '%s'", sender, search_string);

      query = hud_query_new (source, search_string, 10);
      g_dbus_method_invocation_return_value (invocation, describe_query (query));

      g_ptr_array_add(query_list, query);
      g_object_weak_ref(G_OBJECT(query), query_destroyed, query_list);

      g_variant_unref(vsearch);
    }
  else
    {
      g_warn_if_reached();
    }

  return;
}

/* Gets properties for DBus */
static GVariant *
bus_get_prop (GDBusConnection * connection, const gchar * sender, const gchar * object_path, const gchar * interface_name, const gchar * property_name, GError ** error, gpointer user_data)
{
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
	} else {
		g_warn_if_reached();
	}

	return NULL;
}

static GMainLoop *mainloop = NULL;
static GDBusInterfaceVTable vtable = {
	.method_call = bus_method,
	.get_property = bus_get_prop,
	.set_property = NULL
};

static void
bus_acquired_cb (GDBusConnection *connection,
                 const gchar     *name,
                 gpointer         user_data)
{
  HudSource *source = user_data;
  GError *error = NULL;

  g_debug ("Bus acquired (guid %s)", g_dbus_connection_get_guid (connection));

  if (!g_dbus_connection_register_object (connection, DBUS_PATH, hud_iface_com_canonical_hud_interface_info (), &vtable, source, NULL, &error))
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
  HudWindowSource *window_source;
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

  /* we will eventually pull GtkMenu out of this, so keep it around */
  window_source = hud_window_source_new ();
  hud_source_list_add (source_list, HUD_SOURCE (window_source));

  {
    HudIndicatorSource *source;

    source = hud_indicator_source_new ();
    hud_source_list_add (source_list, HUD_SOURCE (source));
    g_object_unref (source);
  }

  {
    HudAppIndicatorSource *source;

    source = hud_app_indicator_source_new ();
    hud_source_list_add (source_list, HUD_SOURCE (source));
    g_object_unref (source);
  }
  
  {
    HudWebappSource *source;
    
    source = hud_webapp_source_new (window_source);
    hud_source_list_add (source_list, HUD_SOURCE (source));
    
    g_object_unref (G_OBJECT (source));
  }

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
  
  g_main_loop_run (mainloop);
  g_main_loop_unref (mainloop);

  g_object_unref (window_source);
  g_object_unref (source_list);
  g_ptr_array_free(query_list, TRUE);

  return 0;
}
