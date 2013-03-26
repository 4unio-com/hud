/*
Test code for watchdog

Copyright 2013 Canonical Ltd.

Authors:
    Ted Gould <ted@canonical.com>

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

#include "watchdog.h"

static void
test_watchdog_create (void)
{
	g_unsetenv("HUD_SERVICE_TIMEOUT");

	/* Try with NULL */
	HudWatchdog * doggie = hud_watchdog_new(NULL);

	g_assert(IS_HUD_WATCHDOG(doggie));
	g_assert(hud_watchdog_get_timeout(doggie) == 600);

	g_clear_object(&doggie);

	/* Give it a loop */
	GMainLoop * loop = g_main_loop_new(NULL, FALSE);
	doggie = hud_watchdog_new(loop);

	g_assert(IS_HUD_WATCHDOG(doggie));
	g_assert(hud_watchdog_get_timeout(doggie) == 600);

	g_clear_object(&doggie);
	g_main_loop_unref(loop);

	/* Set the environment variable */
	g_setenv("HUD_SERVICE_TIMEOUT", "1000", TRUE);

	doggie = hud_watchdog_new(NULL);

	g_assert(IS_HUD_WATCHDOG(doggie));
	g_assert(hud_watchdog_get_timeout(doggie) == 1000);

	g_clear_object(&doggie);

	return;
}

static gboolean
final_fail (gpointer ploop)
{
	g_error("Timeout not via the watchdog.  It didn't work.");
	g_main_loop_quit((GMainLoop *)ploop);
	return FALSE;
}

static void
test_watchdog_timing (void)
{
	HudWatchdog * doggie = NULL;
	GMainLoop * loop = NULL;

	g_setenv("HUD_SERVICE_TIMEOUT", "1", TRUE);

	loop = g_main_loop_new(NULL, FALSE);
	doggie = hud_watchdog_new(loop);

	glong final = g_timeout_add_seconds(5, final_fail, loop);
	g_main_loop_run(loop);
	g_source_remove(final);

	g_main_loop_unref(loop);
	g_clear_object(&doggie);

	return;
}

/* Build the test suite */
static void
test_watchdog_suite (void)
{
	g_test_add_func ("/hud/watchdog/create",   test_watchdog_create);
	g_test_add_func ("/hud/watchdog/timing",   test_watchdog_timing);
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
	test_watchdog_suite();

	return g_test_run ();
}
