/*
Copyright 2012 Canonical Ltd.

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

#include <glib-object.h>
#include <libdbusmenu-glib/server.h>
#include <libdbusmenu-jsonloader/json-loader.h>

int
main (int argv, char ** argc)
{
	if (argv != 4) {
		g_print("'%s <DBus name> <Object Path> <JSON file>' is how you should use this program.\n", argc[0]);
		return 1;
	}

	g_type_init();

	DbusmenuMenuitem * root = dbusmenu_json_build_from_file(argc[3]);
	g_return_val_if_fail(root != NULL, FALSE);

	DbusmenuServer * server = dbusmenu_server_new(argc[2]);
	dbusmenu_server_set_root(server, root);

	g_bus_own_name(G_BUS_TYPE_SESSION, argc[1], 0, NULL, NULL, NULL, NULL, NULL);

	GMainLoop * mainloop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(mainloop);

	return 0;
}
