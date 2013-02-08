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
 *
 * Author: Ted Gould <ted@canonical.com>
 */

#if !defined (_HUD_CLIENT_H_INSIDE) && !defined (HUD_CLIENT_COMPILATION)
#error "Only <hud-client.h> can be included directly."
#endif

#ifndef __HUD_CLIENT_CONNECTION_H__
#define __HUD_CLIENT_CONNECTION_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define HUD_CLIENT_TYPE_CONNECTION            (hud_client_connection_get_type ())
#define HUD_CLIENT_CONNECTION(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), HUD_CLIENT_TYPE_CONNECTION, HudClientConnection))
#define HUD_CLIENT_CONNECTION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), HUD_CLIENT_TYPE_CONNECTION, HudClientConnectionClass))
#define HUD_CLIENT_IS_CONNECTION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HUD_CLIENT_TYPE_CONNECTION))
#define HUD_CLIENT_IS_CONNECTION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), HUD_CLIENT_TYPE_CONNECTION))
#define HUD_CLIENT_CONNECTION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), HUD_CLIENT_TYPE_CONNECTION, HudClientConnectionClass))

#define HUD_CLIENT_CONNECTION_SIGNAL_CONNECTION_STATUS   "connection-status"

typedef struct _HudClientConnection         HudClientConnection;
typedef struct _HudClientConnectionClass    HudClientConnectionClass;
typedef struct _HudClientConnectionPrivate  HudClientConnectionPrivate ;

/**
 * HudClientConnectionClass:
 * @parent_class: #GObjectClass
 *
 * Class information for #HudClientConnection
 */
struct _HudClientConnectionClass {
	GObjectClass parent_class;
};

/**
 * HudClientConnection:
 *
 * Object to make a generic connection to a HUD service.  For the most
 * part people should just create a #HudClientQuery and that'll use the
 * default HUD service.  For most folks that should be enough.
 */
struct _HudClientConnection {
	GObject parent;
	HudClientConnectionPrivate * priv;
};

GType                   hud_client_connection_get_type   (void);
HudClientConnection *   hud_client_connection_get_ref    (void);
HudClientConnection *   hud_client_connection_new        (gchar * dbus_address,
                                                          gchar * dbus_path);
gboolean                hud_client_connection_new_query  (HudClientConnection * connection,
                                                          const gchar * query,
                                                          gchar ** query_path,
                                                          gchar ** results_name,
                                                          gchar ** appstack_name);
const gchar *           hud_client_connection_get_address (HudClientConnection * connection);
gboolean                hud_client_connection_connected  (HudClientConnection * connection);

/**
	SECTION:connection
	@short_description: Provide a connection to the HUD service
	@stability: Unstable
	@include: libhud-client/connection.h

	The connection is an object to maintain a connection to the default
	objects on the HUD service.  It provides access to the functionality
	there and can be used to create queries.

	Most users should not bother with a connection, it will be created
	by the #HudClientQuery if one is not provided.  Most usage is for
	testing and using custom HUD services.
*/

G_END_DECLS

#endif
