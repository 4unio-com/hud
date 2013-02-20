
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

#if !defined (_HUD_CLIENT_H_INSIDE) && !defined (HUD_CLIENT_COMPILATION)
#error "Only <hud-client.h> can be included directly."
#endif

#ifndef __HUD_CLIENT_ENUM_TYPES_H__
#define __HUD_CLIENT_ENUM_TYPES_H__

#include <glib-object.h>

G_BEGIN_DECLS

/* Enumerations from file: "query.h" */
#include "query.h"


GType hud_client_query_toolbar_items_get_type (void) G_GNUC_CONST;
const gchar * hud_client_query_toolbar_items_get_nick (HudClientQueryToolbarItems value) G_GNUC_CONST;
HudClientQueryToolbarItems hud_client_query_toolbar_items_get_value_from_nick (const gchar * nick) G_GNUC_CONST;

/**
	HUD_CLIENT_TYPE_CLIENT_QUERY_TOOLBAR_ITEMS:

	Gets the #GType value for the type associated with the
	#HudClientQueryToolbarItems enumerated type.
*/
#define HUD_CLIENT_TYPE_CLIENT_QUERY_TOOLBAR_ITEMS (hud_client_query_toolbar_items_get_type())


G_END_DECLS

#endif /* __HUD_CLIENT_ENUM_TYPES_H__ */

/* Generated data ends here */

