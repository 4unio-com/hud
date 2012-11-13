#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "hud-client-connection.h"
#include "hud-service-iface.h"

struct _HudClientConnectionPrivate {
	_HudServiceComCanonicalHud * proxy;
};

#define HUD_CLIENT_CONNECTION_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), HUD_CLIENT_TYPE_CONNECTION, HudClientConnectionPrivate))

static void hud_client_connection_class_init (HudClientConnectionClass *klass);
static void hud_client_connection_init       (HudClientConnection *self);
static void hud_client_connection_dispose    (GObject *object);
static void hud_client_connection_finalize   (GObject *object);

G_DEFINE_TYPE (HudClientConnection, hud_client_connection, G_TYPE_OBJECT);

static void
hud_client_connection_class_init (HudClientConnectionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (HudClientConnectionPrivate));

	object_class->dispose = hud_client_connection_dispose;
	object_class->finalize = hud_client_connection_finalize;

	return;
}

static void
hud_client_connection_init (HudClientConnection *self)
{
	self->priv = HUD_CLIENT_CONNECTION_GET_PRIVATE(self);

	GError * error = NULL;
	self->priv->proxy = _hud_service_com_canonical_hud_proxy_new_for_bus_sync(
		G_BUS_TYPE_SESSION,
		G_DBUS_PROXY_FLAGS_NONE,
		"com.canonical.hud",
		"/com/canonical/hud",
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

	G_OBJECT_CLASS (hud_client_connection_parent_class)->dispose (object);
	return;
}

static void
hud_client_connection_finalize (GObject *object)
{

	G_OBJECT_CLASS (hud_client_connection_parent_class)->finalize (object);
	return;
}

HudClientConnection *
hud_client_connection_get_ref (void)
{


	return NULL;
}

HudClientConnection *
hud_client_connection_new (gchar * dbus_address, gchar * dbus_path)
{


	return NULL;
}
