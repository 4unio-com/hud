/*
Copyright 2012 Canonical Ltd.

This program is free software: you can redistribute it and/or modify it 
under the terms of the GNU General Public License version 3, as published 
by the Free Software Foundation.

This program is distributed in the hope that it will be useful, but 
WITHOUT ANY WARRANTY; without even the implied warranties of 
MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR 
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along 
with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

#include <string.h>

#include <libdbustest/dbus-test.h>

#include "huddbusmenucollector.h"
#include "hudsource.h"
#include "hudresult.h"
#include "hudsettings.h"

#define LOADER_NAME  "test.json.loader"
#define LOADER_PATH  "/test/json/loader"

/* hardcode some parameters so the test doesn't fail if the user
 * has bogus things in GSettings.
 */
HudSettings hud_settings = {
	.indicator_penalty = 50,
	.add_penalty = 10,
	.drop_penalty = 10,
	.end_drop_penalty = 1,
	.swap_penalty = 15,
	.max_distance = 30
};

/* If we can't get the name, we should error the test */
static gboolean
name_timeout (gpointer user_data)
{
	g_error("Unable to get name");
	return FALSE;
}

/* Start things up with a basic mock-json-app and wait until it starts */
static void
start_dbusmenu_mock_app (DbusTestService ** service, GDBusConnection ** session, const gchar * jsonfile)
{
	*service = dbus_test_service_new(NULL);

	/* Loader */
	DbusTestProcess * loader = dbus_test_process_new(DBUSMENU_JSON_LOADER);
	dbus_test_process_append_param(loader, LOADER_NAME);
	dbus_test_process_append_param(loader, LOADER_PATH);
	dbus_test_process_append_param(loader, jsonfile);
	dbus_test_task_set_name(DBUS_TEST_TASK(loader), "JSON Loader");
	dbus_test_service_add_task(*service, DBUS_TEST_TASK(loader));
	g_object_unref(loader);

	/* Dummy */
	DbusTestTask * dummy = dbus_test_task_new();
	dbus_test_task_set_wait_for(dummy, LOADER_NAME);
	dbus_test_service_add_task(*service, dummy);
	g_object_unref(dummy);

	/* Setup timeout */
	guint timeout_source = g_timeout_add_seconds(2, name_timeout, NULL);

	/* Get loader up and running and us on that bus */
	g_debug("Starting up Dbusmenu Loader");
	dbus_test_service_start_tasks(*service);

	/* Cleanup timeout */
	g_source_remove(timeout_source);

	/* Set us not to exit when the service goes */
	*session = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, NULL);
	g_dbus_connection_set_exit_on_close(*session, FALSE);

	return;
}

/* Gets called for each item in the collector, there should be only one */
static void
test_menus_dbusmenu_base_search (HudResult * result, gpointer user_data)
{
	g_assert(result != NULL);
	g_assert(HUD_IS_RESULT(result));

	HudItem * item = hud_result_get_item(result);
	g_assert(item != NULL);
	g_assert(HUD_IS_ITEM(item));

	g_assert(g_strcmp0(hud_item_get_app_icon(item), "no-icon") == 0);
	g_assert(g_strcmp0(hud_item_get_command(item), "Simple") == 0);

	g_object_unref(result);

	gboolean * found = (gboolean *)user_data;
	*found = TRUE;

	return;
}

/* Timeout on our loop */
static gboolean
test_menus_dbusmenu_timeout (gpointer user_data)
{
	GMainLoop * loop = (GMainLoop *)user_data;
	g_main_loop_quit(loop);
	return FALSE;
}

/* Create a basic dbusmenu item and make sure we can get it through
   the collector */
static void
test_menus_dbusmenu_base (void) 
{
	DbusTestService * service = NULL;
	GDBusConnection * session = NULL;

	start_dbusmenu_mock_app(&service, &session, JSON_SIMPLE);

	HudDbusmenuCollector * collector = hud_dbusmenu_collector_new_for_endpoint("test-id",
	                                                                           "Prefix",
	                                                                           "no-icon",
	                                                                           0, /* penalty */
	                                                                           LOADER_NAME,
	                                                                           LOADER_PATH);
	g_assert(collector != NULL);
	g_assert(HUD_IS_DBUSMENU_COLLECTOR(collector));

	GMainLoop * temploop = g_main_loop_new(NULL, FALSE);
	g_timeout_add(100, test_menus_dbusmenu_timeout, temploop);
	g_main_loop_run(temploop);
	g_main_loop_unref(temploop);

	hud_source_use(HUD_SOURCE(collector));

	gboolean found = FALSE;
	hud_source_search(HUD_SOURCE(collector), NULL, test_menus_dbusmenu_base_search, &found);

	g_assert(found);

	hud_source_unuse(HUD_SOURCE(collector));

	g_object_unref(service);
	g_object_unref(session);

	return;
}

struct {
	gchar * label;
	gchar * shortcut;
} shortcutdb[] = {
	{.label = "Save",      .shortcut = "Ctrl + S"},
	{.label = NULL,        .shortcut = NULL}
};

/* Gets called for each item in the collector, there should be only one */
static void
test_menus_dbusmenu_shortcut_search (HudResult * result, gpointer user_data)
{
	g_assert(result != NULL);
	g_assert(HUD_IS_RESULT(result));

	HudItem * item = hud_result_get_item(result);
	g_assert(item != NULL);
	g_assert(HUD_IS_ITEM(item));

	gint * entry = (gint *)user_data;
	g_assert(*entry != -1);

	g_assert(g_strcmp0(hud_item_get_app_icon(item), "no-icon") == 0);
	g_assert(g_strcmp0(hud_item_get_command(item), shortcutdb[*entry].label) == 0);
	g_assert(g_strcmp0(hud_item_get_shortcut(item), shortcutdb[*entry].shortcut) == 0);

	g_object_unref(result);

	*entry = -1;

	return;
}


/* Create a basic dbusmenu item and make sure we can get it through
   the collector */
static void
test_menus_dbusmenu_shortcuts (void) 
{
	DbusTestService * service = NULL;
	GDBusConnection * session = NULL;

	start_dbusmenu_mock_app(&service, &session, JSON_SHORTCUTS);

	HudDbusmenuCollector * collector = hud_dbusmenu_collector_new_for_endpoint("test-id",
	                                                                           "Prefix",
	                                                                           "no-icon",
	                                                                           0, /* penalty */
	                                                                           LOADER_NAME,
	                                                                           LOADER_PATH);
	g_assert(collector != NULL);
	g_assert(HUD_IS_DBUSMENU_COLLECTOR(collector));

	GMainLoop * temploop = g_main_loop_new(NULL, FALSE);
	g_timeout_add(100, test_menus_dbusmenu_timeout, temploop);
	g_main_loop_run(temploop);
	g_main_loop_unref(temploop);

	int i;
	for (i = 0; shortcutdb[i].label != NULL; i++) {
		hud_source_use(HUD_SOURCE(collector));

		guint item = i;
		HudTokenList * tl = hud_token_list_new_from_string(shortcutdb[i].label);
		hud_source_search(HUD_SOURCE(collector), tl, test_menus_dbusmenu_shortcut_search, &item);

		g_assert(item == -1);
		hud_token_list_free(tl);

		hud_source_unuse(HUD_SOURCE(collector));
	}

	g_object_unref(service);
	g_object_unref(session);

	return;
}


/* Build the test suite */
static void
test_menu_input_suite (void)
{
	g_test_add_func ("/hud/menus/dbusmenu/base",          test_menus_dbusmenu_base);
	g_test_add_func ("/hud/menus/dbusmenu/shortcuts",     test_menus_dbusmenu_shortcuts);
	return;
}

gint
main (gint argc, gchar * argv[])
{
	g_type_init();

	g_test_init(&argc, &argv, NULL);

	/* Test suites */
	test_menu_input_suite();

	return g_test_run ();
}
