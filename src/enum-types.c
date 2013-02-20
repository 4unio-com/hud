
/* Generated data (by glib-mkenums) */

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
 *
 * Author: Ted Gould <ted@canonical.com>
 */

#include "enum-types.h"

#include "query.h"
/**
	hud_client_query_toolbar_items_get_type:

	Builds a GLib type for the #HudClientQueryToolbarItems enumeration.

	Return value: A unique #GType for the #HudClientQueryToolbarItems enum.
*/
GType
hud_client_query_toolbar_items_get_type (void)
{
	static GType etype = 0;
	if (G_UNLIKELY(etype == 0)) {
		static const GEnumValue values[] = {
			{ HUD_CLIENT_QUERY_TOOLBAR_FULLSCREEN,  "HUD_CLIENT_QUERY_TOOLBAR_FULLSCREEN", "fullscreen" },
			{ HUD_CLIENT_QUERY_TOOLBAR_HELP,  "HUD_CLIENT_QUERY_TOOLBAR_HELP", "help" },
			{ HUD_CLIENT_QUERY_TOOLBAR_PREFERENCES,  "HUD_CLIENT_QUERY_TOOLBAR_PREFERENCES", "preferences" },
			{ HUD_CLIENT_QUERY_TOOLBAR_QUIT,  "HUD_CLIENT_QUERY_TOOLBAR_QUIT", "quit" },
			{ HUD_CLIENT_QUERY_TOOLBAR_UNDO,  "HUD_CLIENT_QUERY_TOOLBAR_UNDO", "undo" },
			{ 0, NULL, NULL}
		};
		
		etype = g_enum_register_static (g_intern_static_string("HudClientQueryToolbarItems"), values);
	}

	return etype;
}

/**
	hud_client_query_toolbar_items_get_nick:
	@value: The value of HudClientQueryToolbarItems to get the nick of

	Looks up in the enum table for the nick of @value.

	Return value: The nick for the given value or #NULL on error
*/
const gchar *
hud_client_query_toolbar_items_get_nick (HudClientQueryToolbarItems value)
{
	GEnumClass * class = G_ENUM_CLASS(g_type_class_ref(hud_client_query_toolbar_items_get_type()));
	g_return_val_if_fail(class != NULL, NULL);

	const gchar * ret = NULL;
	GEnumValue * val = g_enum_get_value(class, value);
	if (val != NULL) {
		ret = val->value_nick;
	}

	g_type_class_unref(class);
	return ret;
}

/**
	hud_client_query_toolbar_items_get_value_from_nick:
	@nick: The enum nick to lookup

	Looks up in the enum table for the value of @nick.

	Return value: The value for the given @nick
*/
HudClientQueryToolbarItems
hud_client_query_toolbar_items_get_value_from_nick (const gchar * nick)
{
	GEnumClass * class = G_ENUM_CLASS(g_type_class_ref(hud_client_query_toolbar_items_get_type()));
	g_return_val_if_fail(class != NULL, 0);

	HudClientQueryToolbarItems ret = 0;
	GEnumValue * val = g_enum_get_value_by_nick(class, nick);
	if (val != NULL) {
		ret = val->value;
	}

	g_type_class_unref(class);
	return ret;
}



/* Generated data ends here */

