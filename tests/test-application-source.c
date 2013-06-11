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

int
main (int argc, char **argv)
{
#ifndef GLIB_VERSION_2_36
  g_type_init ();
#endif

  g_test_init (&argc, &argv, NULL);
  g_test_add_func ("/hud/applicationsource/set_context", test_application_source_set_context);

  return g_test_run ();
}
