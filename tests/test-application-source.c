/*
 * Copyright © 2013 Canonical Ltd.
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
 */

#define G_LOG_DOMAIN "test-application-source"

#include "test-utils.h"
#include "libdbustest/dbus-test.h"

#include "application-source.h"
#include "source.h"

/* Creates a simple source and sets its context */
static void
test_application_source_set_context (void)
{
	DbusTestService * service = dbus_test_service_new(NULL);
	dbus_test_service_start_tasks(service);
	GDBusConnection * con = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, NULL);

	HudApplicationSource * source = hud_application_source_new_for_id("bob");

	g_assert(HUD_IS_APPLICATION_SOURCE(source));
	g_assert_cmpstr(hud_application_source_get_id(source), ==, "bob");

	hud_application_source_set_context(source, 1, "bob-context");
	g_assert_cmpstr(hud_application_source_get_context(source, 1), ==, "bob-context");
	g_assert(hud_application_source_get_context(source, 2) == NULL);

	hud_application_source_set_context(source, 1, NULL);
	g_assert(hud_application_source_get_context(source, 1) == NULL);

	g_object_add_weak_pointer(G_OBJECT(source), (gpointer *)&source);
	g_object_unref(source);
	g_assert(source == NULL);

	g_object_unref(service);
	hud_test_utils_wait_for_connection_close(con);

	return;
}

/* Find the Elephant */
static void
app_source_find_elephant (HudResult * result, gpointer user_data)
{
	g_assert(result != NULL);
	g_assert(HUD_IS_RESULT(result));

	HudItem * item = hud_result_get_item(result);
	g_assert(item != NULL);
	g_assert(HUD_IS_ITEM(item));

	if (g_strcmp0(hud_item_get_command(item), "Elephant") == 0) {
		gboolean * found = (gboolean *)user_data;
		*found = TRUE;
	}

	g_object_unref(result);

	return;
}

/* Find Bush */
static void
app_source_find_bush (HudResult * result, gpointer user_data)
{
	g_assert(result != NULL);
	g_assert(HUD_IS_RESULT(result));

	HudItem * item = hud_result_get_item(result);
	g_assert(item != NULL);
	g_assert(HUD_IS_ITEM(item));

	if (g_strcmp0(hud_item_get_command(item), "Bush") == 0) {
		gboolean * found = (gboolean *)user_data;
		*found = TRUE;
	}

	g_object_unref(result);

	return;
}

/* Find Marble */
static void
app_source_find_marble (HudResult * result, gpointer user_data)
{
	g_assert(result != NULL);
	g_assert(HUD_IS_RESULT(result));

	HudItem * item = hud_result_get_item(result);
	g_assert(item != NULL);
	g_assert(HUD_IS_ITEM(item));

	if (g_strcmp0(hud_item_get_command(item), "Marble") == 0) {
		gboolean * found = (gboolean *)user_data;
		*found = TRUE;
	}

	g_object_unref(result);

	return;
}

/* Creates a simple source and sets its context */
static void
test_application_source_add_context (void)
{
	DbusTestService * service = dbus_test_service_new(NULL);
	dbus_test_service_start_tasks(service);
	GDBusConnection * con = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, NULL);

	HudApplicationSource * source = hud_application_source_new_for_id("bob");
	hud_application_source_set_focused_win(source, 1);

	GSimpleActionGroup * simple_group = g_simple_action_group_new();
	g_action_map_add_action(G_ACTION_MAP(simple_group), G_ACTION(g_simple_action_new("action-name", NULL)));
	GActionGroup * group = G_ACTION_GROUP(simple_group);
	g_assert(group);
	g_assert(g_action_map_lookup_action(G_ACTION_MAP(simple_group), "action-name"));

	GMenu * menu_none = g_menu_new();
	g_menu_append(menu_none, "Elephant", "action-name");
	g_menu_append(menu_none, "Rabbit", "action-name");

	/* Setup a context for the NULL context that has animals */
	HudApplicationSourceContext * context_none = hud_application_source_context_new(1, NULL, "bob", "bob-icon", "/app/bob");
	g_assert(HUD_IS_APPLICATION_SOURCE_CONTEXT(context_none));

	hud_application_source_context_add_action_group(context_none, group, NULL);
	hud_application_source_context_add_model(context_none, G_MENU_MODEL(menu_none), HUD_APPLICATION_SOURCE_CONTEXT_MODEL_WINDOW);

	hud_application_source_add_context(source, context_none);

	g_assert(hud_application_source_has_xid(source, 1));

	/* Search for our animals! */
	hud_source_use(HUD_SOURCE(source));

	gboolean found_elephant = FALSE;
	hud_source_search(HUD_SOURCE(source), NULL, app_source_find_elephant, &found_elephant);

	g_assert(found_elephant);
	hud_source_unuse(HUD_SOURCE(source));

	/* Build some plants */
	GMenu * menu_plants = g_menu_new();
	g_menu_append(menu_plants, "Tree", "plants.action-name");
	g_menu_append(menu_plants, "Flowers", "plants.action-name");
	g_menu_append(menu_plants, "Bush", "plants.action-name");

	HudApplicationSourceContext * context_plants = hud_application_source_context_new(1, "plants", "bob", "bob-icon", "/app/bob");

	hud_application_source_context_add_action_group(context_plants, group, "plants");
	hud_application_source_context_add_model(context_plants, G_MENU_MODEL(menu_plants), HUD_APPLICATION_SOURCE_CONTEXT_MODEL_WINDOW);

	hud_application_source_add_context(source, context_plants);

	/* Build some rocks */
	GMenu * menu_rocks = g_menu_new();
	g_menu_append(menu_rocks, "Marble", "rocks.action-name");

	HudApplicationSourceContext * context_rocks;
	context_rocks = hud_application_source_context_new(0, /* set as ALL_WINDOWS context*/
							   "rocks",
							   "daisy",
							   "daisy-icon",
							   "/app/daisy");

	hud_application_source_context_add_action_group(context_rocks, group, "rocks");
	hud_application_source_context_add_model(context_rocks, G_MENU_MODEL(menu_rocks), HUD_APPLICATION_SOURCE_CONTEXT_MODEL_WINDOW);

	hud_application_source_add_context(source, context_rocks);
	// set rocks as the active context for ALL_WINDOWS
	hud_application_source_set_context(source, 0, "rocks");

	/* Make sure we can still find the elephant */
	hud_source_use(HUD_SOURCE(source));

	found_elephant = FALSE;
	hud_source_search(HUD_SOURCE(source), NULL, app_source_find_elephant, &found_elephant);

	g_assert(found_elephant);
	hud_source_unuse(HUD_SOURCE(source));


	/* Make sure we can find marble */
	hud_source_use(HUD_SOURCE(source));

	gboolean found_marble = FALSE;
	hud_source_search(HUD_SOURCE(source), NULL, app_source_find_marble, &found_marble);

	g_assert(found_marble);
	hud_source_unuse(HUD_SOURCE(source));


	/* Switch to plants and find Bush */
	hud_application_source_set_context(source, 1, "plants");
	hud_source_use(HUD_SOURCE(source));

	gboolean found_bush = FALSE;
	hud_source_search(HUD_SOURCE(source), NULL, app_source_find_bush, &found_bush);

	g_assert(found_bush);
	hud_source_unuse(HUD_SOURCE(source));

	/* Make sure we can still find marble */
	hud_source_use(HUD_SOURCE(source));

	found_marble = FALSE;
	hud_source_search(HUD_SOURCE(source), NULL, app_source_find_marble, &found_marble);

	g_assert(found_marble);
	hud_source_unuse(HUD_SOURCE(source));


	/* Go back and find Elephant */
	hud_application_source_set_context(source, 1, NULL);
	hud_source_use(HUD_SOURCE(source));

	found_elephant = FALSE;
	hud_source_search(HUD_SOURCE(source), NULL, app_source_find_elephant, &found_elephant);

	g_assert(found_elephant);
	hud_source_unuse(HUD_SOURCE(source));

	/* And no Bush */
	hud_source_use(HUD_SOURCE(source));

	found_bush = FALSE;
	hud_source_search(HUD_SOURCE(source), NULL, app_source_find_bush, &found_bush);

	g_assert(!found_bush);
	hud_source_unuse(HUD_SOURCE(source));

	/* Make sure we can find marble */
	hud_source_use(HUD_SOURCE(source));

	found_marble = FALSE;
	hud_source_search(HUD_SOURCE(source), NULL, app_source_find_marble, &found_marble);

	g_assert(found_marble);
	hud_source_unuse(HUD_SOURCE(source));

	/* Cleanup */
	g_object_add_weak_pointer(G_OBJECT(simple_group), (gpointer *)&simple_group);
	g_object_unref(simple_group);
	g_assert(simple_group != NULL);

	g_object_add_weak_pointer(G_OBJECT(menu_none), (gpointer *)&menu_none);
	g_object_unref(menu_none);
	g_assert(menu_none != NULL);

	g_object_add_weak_pointer(G_OBJECT(menu_plants), (gpointer *)&menu_plants);
	g_object_unref(menu_plants);
	g_assert(menu_plants != NULL);

	g_object_add_weak_pointer(G_OBJECT(menu_rocks), (gpointer *)&menu_rocks);
	g_object_unref(menu_rocks);
	g_assert(menu_plants != NULL);

	g_object_unref(source);

	hud_test_utils_process_mainloop(500);

	g_assert(simple_group == NULL);
	g_assert(menu_none == NULL);
	g_assert(menu_plants == NULL);
	g_assert(menu_rocks == NULL);

	g_object_unref(service);
	hud_test_utils_wait_for_connection_close(con);

	return;
}

int
main (int argc, char **argv)
{
#ifndef GLIB_VERSION_2_36
  g_type_init ();
#endif

  g_test_init (&argc, &argv, NULL);
  g_test_add_func ("/hud/applicationsource/set_context", test_application_source_set_context);
  g_test_add_func ("/hud/applicationsource/add_context", test_application_source_add_context);

  return g_test_run ();
}
