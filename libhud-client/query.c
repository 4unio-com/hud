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

#include "query.h"
#include "connection.h"
#include "query-iface.h"

struct _HudClientQueryPrivate {
	_HudQueryComCanonicalHud * proxy;
	HudClientConnection * connection;
	gchar * query;
};

#define HUD_CLIENT_QUERY_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), HUD_CLIENT_TYPE_QUERY, HudClientQueryPrivate))

enum {
	PROP_0 = 0,
	PROP_CONNECTION,
	PROP_QUERY,
};

#define PROP_CONNECTION_S  "connection"
#define PROP_QUERY_S       "query"

static void hud_client_query_class_init  (HudClientQueryClass *klass);
static void hud_client_query_init        (HudClientQuery *self);
static void hud_client_query_constructed (GObject *object);
static void hud_client_query_dispose     (GObject *object);
static void hud_client_query_finalize    (GObject *object);
static void set_property (GObject * obj, guint id, const GValue * value, GParamSpec * pspec);
static void get_property (GObject * obj, guint id, GValue * value, GParamSpec * pspec);

G_DEFINE_TYPE (HudClientQuery, hud_client_query, G_TYPE_OBJECT);

static void
hud_client_query_class_init (HudClientQueryClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (HudClientQueryPrivate));

	object_class->dispose = hud_client_query_dispose;
	object_class->finalize = hud_client_query_finalize;
	object_class->constructed = hud_client_query_constructed;
	object_class->set_property = set_property;
	object_class->get_property = get_property;

	g_object_class_install_property (object_class, PROP_CONNECTION,
	                                 g_param_spec_object(PROP_CONNECTION_S, "Connection to the HUD service",
	                                              "HUD service connection",
	                                              HUD_CLIENT_TYPE_CONNECTION,
	                                              G_PARAM_READABLE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));
	g_object_class_install_property (object_class, PROP_QUERY,
	                                 g_param_spec_string(PROP_QUERY_S, "Query to the HUD service",
	                                              "HUD query",
	                                              NULL,
	                                              G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	return;
}

static void
hud_client_query_init (HudClientQuery *self)
{
	self->priv = HUD_CLIENT_QUERY_GET_PRIVATE(self);

	return;
}

static void
set_property (GObject * obj, guint id, const GValue * value, GParamSpec * pspec)
{
	HudClientQuery * self = HUD_CLIENT_QUERY(obj);

	switch (id) {
	case PROP_CONNECTION:
		g_clear_object(&self->priv->connection);
		self->priv->connection = g_value_dup_object(value);
		break;
	case PROP_QUERY:
		hud_client_query_set_query(self, g_value_get_string(value));
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
	HudClientQuery * self = HUD_CLIENT_QUERY(obj);

	switch (id) {
	case PROP_CONNECTION:
		g_value_set_object(value, self->priv->connection);
		break;
	case PROP_QUERY:
		g_value_set_string(value, self->priv->query);
		break;
	default:
		g_warning("Unknown property %d.", id);
		return;
	}

	return;
}

static void
hud_client_query_constructed (GObject *object)
{
	HudClientQuery * self = HUD_CLIENT_QUERY(object);

	G_OBJECT_CLASS (hud_client_query_parent_class)->constructed (object);

	if (self->priv->connection == NULL) {
		self->priv->connection = hud_client_connection_get_ref();
	}

	return;
}

static void
hud_client_query_dispose (GObject *object)
{
	HudClientQuery * self = HUD_CLIENT_QUERY(object);

	g_clear_object(&self->priv->proxy);
	g_clear_object(&self->priv->connection);

	G_OBJECT_CLASS (hud_client_query_parent_class)->dispose (object);
	return;
}

static void
hud_client_query_finalize (GObject *object)
{
	HudClientQuery * self = HUD_CLIENT_QUERY(object);

	g_clear_pointer(&self->priv->query, g_free);

	G_OBJECT_CLASS (hud_client_query_parent_class)->finalize (object);
	return;
}

HudClientQuery *
hud_client_query_new (const gchar * query)
{
	return HUD_CLIENT_QUERY(g_object_new(HUD_CLIENT_TYPE_QUERY,
		PROP_QUERY_S, query,
		NULL
	));
}

HudClientQuery *
hud_client_query_new_for_connection (const gchar * query, HudClientConnection * connection)
{
	return HUD_CLIENT_QUERY(g_object_new(HUD_CLIENT_TYPE_QUERY,
		PROP_CONNECTION_S, connection,
		PROP_QUERY_S, query,
		NULL
	));
}

void
hud_client_query_set_query (HudClientQuery * cquery, const gchar * query)
{
	g_return_if_fail(HUD_CLIENT_IS_QUERY(cquery));

	g_clear_pointer(&cquery->priv->query, g_free);
	cquery->priv->query = g_strdup(query);

	if (cquery->priv->proxy != NULL) {
		gint revision = 0;
		_hud_query_com_canonical_hud_call_update_query_sync(cquery->priv->proxy, cquery->priv->query, &revision, NULL, NULL);
		g_debug("Revision for update: %d", revision);
	} else {
		gchar * path = NULL;
		gchar * results = NULL;
		
		/* This is perhaps a little extreme, but really, if this is failing
		   there's a whole world of hurt for us. */
		g_return_if_fail(hud_client_connection_new_query(cquery->priv->connection, cquery->priv->query, &path, &results));

		cquery->priv->proxy = _hud_query_com_canonical_hud_proxy_new_for_bus_sync(
			G_BUS_TYPE_SESSION,
			G_DBUS_PROXY_FLAGS_NONE,
			hud_client_connection_get_address(cquery->priv->connection),
			path,
			NULL, /* GCancellable */
			NULL  /* GError */
		);

		/* TODO: Make a DeeModel */

		g_free(path);
		g_free(results);
	}

	g_object_notify(G_OBJECT(cquery), PROP_QUERY_S);

	return;
}

const gchar *
hud_client_query_get_query (HudClientQuery * cquery)
{
	g_return_val_if_fail(HUD_CLIENT_IS_QUERY(cquery), NULL);

	return cquery->priv->query;
}

DeeModel *
hud_client_query_get_results_model (HudClientQuery * cquery)
{


	return NULL;
}
