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

#include "application-source.h"

/* Creates a simple source and sets its context */
static void
test_application_source_set_context (void)
{
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

	return;
}

/* Creates a simple source and sets its context */
static void
test_application_source_add_context (void)
{
	HudApplicationSource * source = hud_application_source_new_for_id("bob");

	GSimpleActionGroup * simple_group = g_simple_action_group_new();
	g_simple_action_group_insert(simple_group, G_ACTION(g_simple_action_new("action-name", NULL)));
	GActionGroup * group = G_ACTION_GROUP(simple_group);
	g_assert(group);
	g_assert(g_simple_action_group_lookup(simple_group, "action-name"));

	GMenu * menu_none = g_menu_new();
	g_menu_append(menu_none, "Elephant", "action-name");
	g_menu_append(menu_none, "Rabbit", "action-name");

	/* Setup a context for the NULL context that has animals */
	HudApplicationSourceContext * context_none = hud_application_source_context_new(1, NULL, "bob", "bob-icon", "/app/bob");
	g_assert(HUD_IS_APPLICATION_SOURCE_CONTEXT(context_none));

	hud_application_source_context_add_model(context_none, G_MENU_MODEL(menu_none));
	hud_application_source_context_add_action_group(context_none, group, NULL);

	hud_application_source_add_context(source, context_none);

	g_assert(hud_application_source_has_xid(source, 1));

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
