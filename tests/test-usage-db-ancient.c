/*
Test function to ensure we auto-clear the old databse

Copyright 2011 Canonical Ltd.

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

#include "../service/load-app-info.h"
#include "../service/load-app-info.c"
#include "../service/usage-tracker.h"
#include "../service/usage-tracker.c"

gint
main (gint argc, gchar * argv[])
{
	g_type_init();
	UsageTracker * tracker = usage_tracker_new();
	g_object_unref(tracker);
	return 0;
}
