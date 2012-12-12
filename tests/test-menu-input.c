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

#include <string.h>

#include "hudresult.h"
#include "hudsettings.h"

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


/* Build the test suite */
static void
test_menu_input_suite (void)
{
	// g_test_add_func ("/hud/menus/dbusmenu/base",          test_menus_dbusmenu_base);
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
