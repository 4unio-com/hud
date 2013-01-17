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

#include "hudsettings.h"
#include "load-app-info.h"
#include "usage-tracker.h"

HudSettings hud_settings = {
  .store_usage_data = TRUE
};

gint
main (gint argc, gchar * argv[])
{
	if (!GLIB_CHECK_VERSION(2, 35, 0))
		g_type_init (); /* Only needed in versions < 2.35.0 */

	UsageTracker * tracker = usage_tracker_new();
	g_object_unref(tracker);
	return 0;
}
