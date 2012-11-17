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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "connection.h"
#include "service-iface.h"

#include "shared-values.h"

struct _HudClientConnectionPrivate {
	_HudServiceComCanonicalHud * proxy;
	gchar * address;
	gchar * path;
};

#define HUD_CLIENT_CONNECTION_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), HUD_CLIENT_TYPE_CONNECTION, HudClientConnectionPrivate))

enum {
	PROP_0 = 0,
	PROP_ADDRESS,
	PROP_PATH,
};

#define PROP_ADDRESS_S  "address"
#define PROP_PATH_S     "path"

static void hud_client_connection_class_init (HudClientConnectionClass *klass);
static void hud_client_connection_init       (HudClientConnection *self);
static void hud_client_connection_constructed (GObject *object);
static void hud_client_connection_dispose    (GObject *object);
static void hud_client_connection_finalize   (GObject *object);
static void set_property (GObject * obj, guint id, const GValue * value, GParamSpec * pspec);
static void get_property (GObject * obj, guint id, GValue * value, GParamSpec * pspec);

G_DEFINE_TYPE (HudClientConnection, hud_client_connection, G_TYPE_OBJECT);

static void
hud_client_connection_class_init (HudClientConnectionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (HudClientConnectionPrivate));

	object_class->dispose = hud_client_connection_dispose;
	object_class->finalize = hud_client_connection_finalize;
	object_class->constructed = hud_client_connection_constructed;
	object_class->set_property = set_property;
	object_class->get_property = get_property;

	g_object_class_install_property (object_class, PROP_ADDRESS,
	                                 g_param_spec_string(PROP_ADDRESS_S, "Address on DBus for the HUD service",
	                                              "The DBus address of the HUD service we should connect to.",
	                                              DBUS_NAME,
	                                              G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

	g_object_class_install_property (object_class, PROP_ADDRESS,
	                                 g_param_spec_string(PROP_PATH_S, "Path on DBus for the HUD service",
	                                              "The DBus path of the HUD service we should connect to.",
	                                              DBUS_PATH,
	                                              G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

	return;
}

static void
hud_client_connection_init (HudClientConnection *self)
{
	self->priv = HUD_CLIENT_CONNECTION_GET_PRIVATE(self);

	return;
}

static void
set_property (GObject * obj, guint id, const GValue * value, GParamSpec * pspec)
{
	HudClientConnection * self = HUD_CLIENT_CONNECTION(obj);

	switch (id) {
	case PROP_ADDRESS:
		g_clear_pointer(&self->priv->address, g_free);
		self->priv->address = g_value_dup_string(value);
		break;
	case PROP_PATH:
		g_clear_pointer(&self->priv->path, g_free);
		self->priv->path = g_value_dup_string(value);
		break;
	default:
		g_warning("Unknown property %d.", id);
		return;
	}

	return;
}

static void
get_property (GObject * obj, guint id, GValue * value, GParamSpec * pspec)
{
	HudClientConnection * self = HUD_CLIENT_CONNECTION(obj);

	switch (id) {
	case PROP_ADDRESS:
		g_value_set_string(value, self->priv->address);
		break;
	case PROP_PATH:
		g_value_set_string(value, self->priv->path);
		break;
	default:
		g_warning("Unknown property %d.", id);
		return;
	}

	return;
}

static void
hud_client_connection_constructed (GObject * object)
{
	HudClientConnection * self = HUD_CLIENT_CONNECTION(object);

	GError * error = NULL;
	self->priv->proxy = _hud_service_com_canonical_hud_proxy_new_for_bus_sync(
		G_BUS_TYPE_SESSION,
		G_DBUS_PROXY_FLAGS_NONE,
		self->priv->address,
		self->priv->path,
		NULL, /* GCancellable */
		&error
	);

	if (error != NULL) {
		g_warning("Unable to get a HUD proxy: %s", error->message);
		self->priv->proxy = NULL;
		g_error_free(error); error = NULL;
	}

	return;
}

static void
hud_client_connection_dispose (GObject *object)
{
	HudClientConnection * self = HUD_CLIENT_CONNECTION(object);

	g_clear_object(&self->priv->proxy);

	G_OBJECT_CLASS (hud_client_connection_parent_class)->dispose (object);
	return;
}

static void
hud_client_connection_finalize (GObject *object)
{
	HudClientConnection * self = HUD_CLIENT_CONNECTION(object);

	g_clear_pointer(&self->priv->address, g_free);
	g_clear_pointer(&self->priv->path, g_free);

	G_OBJECT_CLASS (hud_client_connection_parent_class)->finalize (object);
	return;
}

HudClientConnection *
hud_client_connection_get_ref (void)
{
	static HudClientConnection * global = NULL;

	if (global == NULL) {
		global = HUD_CLIENT_CONNECTION(g_object_new(HUD_CLIENT_TYPE_CONNECTION, NULL));
		g_object_add_weak_pointer(G_OBJECT(global), (gpointer *)&global);
		return global;
	} else {
		return g_object_ref(global);
	}
}

HudClientConnection *
hud_client_connection_new (gchar * dbus_address, gchar * dbus_path)
{
	return HUD_CLIENT_CONNECTION(g_object_new(HUD_CLIENT_TYPE_CONNECTION,
			PROP_ADDRESS_S, dbus_address,
			PROP_PATH_S, dbus_path,
			NULL));
}

gboolean
hud_client_connection_new_query (HudClientConnection * connection, const gchar * query, gchar ** query_path, gchar ** results_path)
{
	g_return_val_if_fail(HUD_CLIENT_IS_CONNECTION(connection), FALSE);

	gint modelrev = 0;

	return _hud_service_com_canonical_hud_call_start_query_sync(connection->priv->proxy,
		query,
		query_path,
		results_path,
		&modelrev,
		NULL,  /* GCancellable */
		NULL); /* GError */
}

const gchar *
hud_client_connection_get_address (HudClientConnection * connection)
{
	g_return_val_if_fail(HUD_CLIENT_IS_CONNECTION(connection), NULL);

	return connection->priv->address;
}
