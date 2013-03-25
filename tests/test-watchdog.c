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

	return;
}

/* Build the test suite */
static void
test_watchdog_suite (void)
{
	g_test_add_func ("/hud/watchdog/create",   test_watchdog_create);
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
