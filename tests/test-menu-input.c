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
#include "hudmenumodelcollector.h"
#include "hudsource.h"
#include "hudresult.h"
#include "hudsettings.h"
#include "hudtestutils.h"

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

/* Create a basic dbusmenu item and make sure we can get it through
   the collector */
static void
test_menus_dbusmenu_base (void) 
{
  hud_test_utils_ignore_dbus_null_connection();

	DbusTestService * service = NULL;
	GDBusConnection * session = NULL;

	hud_test_utils_start_dbusmenu_mock_app(&service, &session, JSON_SIMPLE);

	HudDbusmenuCollector * collector = hud_dbusmenu_collector_new_for_endpoint("test-id",
	                                                                           "Prefix",
	                                                                           "no-icon",
	                                                                           0, /* penalty */
	                                                                           HUD_TEST_UTILS_LOADER_NAME,
	                                                                           HUD_TEST_UTILS_LOADER_PATH);
	g_assert(collector != NULL);
	g_assert(HUD_IS_DBUSMENU_COLLECTOR(collector));

	hud_test_utils_process_mainloop(100);

	hud_source_use(HUD_SOURCE(collector));

	gboolean found = FALSE;
	hud_source_search(HUD_SOURCE(collector), NULL, test_menus_dbusmenu_base_search, &found);

	g_assert(found);

	hud_source_unuse(HUD_SOURCE(collector));

	g_object_unref(collector);
	g_object_unref(service);

	hud_test_utils_wait_for_connection_close(session);
}

struct {
	gchar * label;
	gchar * shortcut;
	gboolean model_support;
} shortcutdb[] = {
	{.label = "Save",      .shortcut = "Ctrl + S",            .model_support = TRUE},
	{.label = "Quiter",    .shortcut = "Ctrl + Alt + Q",      .model_support = TRUE},
	{.label = "Emacs",     .shortcut = "Ctrl + X, Ctrl + W",  .model_support = FALSE},
	{.label = "Close",     .shortcut = "Super + W",           .model_support = TRUE},
	{.label = "Nothing",   .shortcut = "",                    .model_support = TRUE},
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
  hud_test_utils_ignore_dbus_null_connection();

	DbusTestService * service = NULL;
	GDBusConnection * session = NULL;

	hud_test_utils_start_dbusmenu_mock_app(&service, &session, JSON_SHORTCUTS);

	hud_test_utils_process_mainloop(100);

	HudDbusmenuCollector * collector = hud_dbusmenu_collector_new_for_endpoint("test-id",
	                                                                           "Prefix",
	                                                                           "no-icon",
	                                                                           0, /* penalty */
	                                                                           HUD_TEST_UTILS_LOADER_NAME,
	                                                                           HUD_TEST_UTILS_LOADER_PATH);
	g_assert(collector != NULL);
	g_assert(HUD_IS_DBUSMENU_COLLECTOR(collector));

	hud_test_utils_process_mainloop(100);

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

	g_object_unref(collector);
	g_object_unref(service);

	hud_test_utils_wait_for_connection_close(session);
}

/* Find an item in the base menu model */
static void
test_menus_model_base (void) 
{
	DbusTestService * service = NULL;
	GDBusConnection * session = NULL;

	hud_test_utils_start_model_mock_app(&service, &session, MODEL_SIMPLE);

	HudMenuModelCollector * collector = hud_menu_model_collector_new("test-id",
	                                                                 "no-icon",
	                                                                 0); /* penalty */

	g_assert(collector != NULL);
	g_assert(HUD_IS_MENU_MODEL_COLLECTOR(collector));

	hud_test_utils_process_mainloop(100);
	
	hud_menu_model_collector_add_endpoint(collector,
	                                      "Prefix",
	                                      HUD_TEST_UTILS_LOADER_NAME,
	                                      HUD_TEST_UTILS_LOADER_PATH,
	                                      HUD_TEST_UTILS_LOADER_PATH);

	hud_test_utils_process_mainloop(100);

	hud_source_use(HUD_SOURCE(collector));

	gboolean found = FALSE;
	hud_source_search(HUD_SOURCE(collector), NULL, test_menus_dbusmenu_base_search, &found);

	g_assert(found);

	hud_source_unuse(HUD_SOURCE(collector));

	g_object_unref(collector);
	g_object_unref(service);

	hud_test_utils_wait_for_connection_close(session);
}

/* Create model items with various shortcuts */
static void
test_menus_model_shortcuts (void) 
{
	DbusTestService * service = NULL;
	GDBusConnection * session = NULL;

	hud_test_utils_start_model_mock_app(&service, &session, MODEL_SHORTCUTS);

	HudMenuModelCollector * collector = hud_menu_model_collector_new("test-id",
	                                                                 "no-icon",
	                                                                 0); /* penalty */

	g_assert(collector != NULL);
	g_assert(HUD_IS_MENU_MODEL_COLLECTOR(collector));

	hud_test_utils_process_mainloop(100);

	hud_menu_model_collector_add_endpoint(collector,
	                                      "Prefix",
	                                      HUD_TEST_UTILS_LOADER_NAME,
	                                      HUD_TEST_UTILS_LOADER_PATH,
	                                      HUD_TEST_UTILS_LOADER_PATH);

	hud_test_utils_process_mainloop(100);
	
	int i;
	for (i = 0; shortcutdb[i].label != NULL; i++) {
		if (!shortcutdb[i].model_support) {
			continue;
		}

		hud_source_use(HUD_SOURCE(collector));

		guint item = i;
		HudTokenList * tl = hud_token_list_new_from_string(shortcutdb[i].label);
		hud_source_search(HUD_SOURCE(collector), tl, test_menus_dbusmenu_shortcut_search, &item);

		g_assert(item == -1);
		hud_token_list_free(tl);

		hud_source_unuse(HUD_SOURCE(collector));
	}

	g_object_unref(collector);
	g_object_unref(service);

	hud_test_utils_wait_for_connection_close(session);
}

/* Gets called for each item in the collector, there should be only one */
static void
test_menus_model_deep_search (HudResult * result, gpointer user_data)
{
	g_assert(result != NULL);
	g_assert(HUD_IS_RESULT(result));

	g_object_unref(result);

	gboolean * found = (gboolean *)user_data;
	*found = TRUE;

	return;
}

/* Create model items with various shortcuts */
static void
test_menus_model_deep (void) 
{
	DbusTestService * service = NULL;
	GDBusConnection * session = NULL;

	hud_test_utils_start_model_mock_app(&service, &session, MODEL_DEEP);

	HudMenuModelCollector * collector = hud_menu_model_collector_new("test-id",
	                                                                 "no-icon",
	                                                                 0); /* penalty */

	g_assert(collector != NULL);
	g_assert(HUD_IS_MENU_MODEL_COLLECTOR(collector));

	hud_test_utils_process_mainloop(100);

	hud_menu_model_collector_add_endpoint(collector,
	                                      "Prefix",
	                                      HUD_TEST_UTILS_LOADER_NAME,
	                                      HUD_TEST_UTILS_LOADER_PATH);

	hud_test_utils_process_mainloop(100);
	
	hud_source_use(HUD_SOURCE(collector));

	gboolean found = FALSE;

	/* Check for an item on the first level so we know things are working */
	HudTokenList * tl = hud_token_list_new_from_string("Base");
	hud_source_search(HUD_SOURCE(collector), tl, test_menus_model_deep_search, &found);

	g_assert(found);
	hud_token_list_free(tl);

	/* Then check to make sure we can't get one too deep */
	found = FALSE;
	tl = hud_token_list_new_from_string("Eleven");
	hud_source_search(HUD_SOURCE(collector), tl, test_menus_model_deep_search, &found);

	g_assert(!found);
	hud_token_list_free(tl);

	hud_source_unuse(HUD_SOURCE(collector));

	g_object_unref(collector);
	g_object_unref(service);
	g_object_unref(session);

	hud_test_utils_process_mainloop(100);
}


/* Build the test suite */
static void
test_menu_input_suite (void)
{
	g_test_add_func ("/hud/menus/dbusmenu/base",          test_menus_dbusmenu_base);
	g_test_add_func ("/hud/menus/dbusmenu/shortcuts",     test_menus_dbusmenu_shortcuts);
	g_test_add_func ("/hud/menus/model/base",             test_menus_model_base);
	g_test_add_func ("/hud/menus/model/shortcuts",        test_menus_model_shortcuts);
	g_test_add_func ("/hud/menus/model/deep",             test_menus_model_deep);

	return;
}

gint
main (gint argc, gchar * argv[])
{
#ifndef GLIB_VERSION_2_36
  g_type_init ();
#endif

	g_test_init(&argc, &argv, NULL);

	/* Test suites */
	test_menu_input_suite();

	return g_test_run ();
}
