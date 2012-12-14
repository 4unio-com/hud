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
#include <gio/gio.h>

int
main (int argv, char ** argc)
{
	if (argv != 3) {
		g_print("'%s <DBus name> <Object Path>' is how you should use this program.\n", argc[0]);
		return 1;
	}

	g_type_init();

	GMenu * menu = g_menu_new();
	GMenuItem * mi = NULL;

	mi = g_menu_item_new("Save", "save");
	g_menu_item_set_attribute(mi, "accel", "<Control>S");
	g_menu_append_item(menu, mi);

	mi = g_menu_item_new("Quiter", "quiter");
	g_menu_item_set_attribute(mi, "accel", "<Primary><Alt>Q");
	g_menu_append_item(menu, mi);

	mi = g_menu_item_new("Close", "close");
	g_menu_item_set_attribute(mi, "accel", "<Super>W");
	g_menu_append_item(menu, mi);

	GSimpleActionGroup * ag = g_simple_action_group_new();
	g_simple_action_group_insert(ag, G_ACTION(g_simple_action_new("save", G_VARIANT_TYPE_BOOLEAN)));
	g_simple_action_group_insert(ag, G_ACTION(g_simple_action_new("quiter", G_VARIANT_TYPE_BOOLEAN)));
	g_simple_action_group_insert(ag, G_ACTION(g_simple_action_new("close", G_VARIANT_TYPE_BOOLEAN)));

	GDBusConnection * session = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, NULL);

	g_dbus_connection_export_action_group(session, argc[2], G_ACTION_GROUP(ag), NULL);
	g_dbus_connection_export_menu_model(session, argc[2], G_MENU_MODEL(menu), NULL);

	g_bus_own_name(G_BUS_TYPE_SESSION, argc[1], 0, NULL, NULL, NULL, NULL, NULL);

	GMainLoop * mainloop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(mainloop);

	return 0;
}
