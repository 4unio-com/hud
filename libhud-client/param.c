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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "param.h"
#include <gio/gio.h>

struct _HudClientParamPrivate {
	GDBusConnection * session;

	gchar * dbus_address;
	gchar * base_action;
	gchar * action_path;
	gchar * model_path;
	gint model_section;
};

#define HUD_CLIENT_PARAM_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), HUD_CLIENT_TYPE_PARAM, HudClientParamPrivate))

static void hud_client_param_class_init (HudClientParamClass *klass);
static void hud_client_param_init       (HudClientParam *self);
static void hud_client_param_dispose    (GObject *object);
static void hud_client_param_finalize   (GObject *object);

G_DEFINE_TYPE (HudClientParam, hud_client_param, G_TYPE_OBJECT);

static void
hud_client_param_class_init (HudClientParamClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (HudClientParamPrivate));

	object_class->dispose = hud_client_param_dispose;
	object_class->finalize = hud_client_param_finalize;

	return;
}

static void
hud_client_param_init (HudClientParam *self)
{
	self->priv = HUD_CLIENT_PARAM_GET_PRIVATE(self);

	self->priv->session = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, NULL);

	return;
}

static void
hud_client_param_dispose (GObject *object)
{
	HudClientParam * param = HUD_CLIENT_PARAM(object);

	g_clear_object(&param->priv->session);

	G_OBJECT_CLASS (hud_client_param_parent_class)->dispose (object);
	return;
}

static void
hud_client_param_finalize (GObject *object)
{
	HudClientParam * param = HUD_CLIENT_PARAM(object);

	g_clear_pointer(&param->priv->base_action, g_free);
	g_clear_pointer(&param->priv->action_path, g_free);
	g_clear_pointer(&param->priv->model_path, g_free);

	G_OBJECT_CLASS (hud_client_param_parent_class)->finalize (object);
	return;
}

/**
 * hud_client_param_new:
 * @dbus_address: The address on dbus to find the actions
 * @base_action: The action to send events for the dialog on
 * @action_path: DBus path to the action object
 * @model_path: DBus path to the menu model object
 * @model_section: Section of the model to use
 *
 * Create a new #HudClientParam object for adjusting a specified
 * paramaterized dialog.
 *
 * Return value: (transfer full): A new #HudClientParam dialog
 */
HudClientParam *
hud_client_param_new (const gchar * dbus_address, const gchar * base_action, const gchar * action_path, const gchar * model_path, gint model_section)
{
	g_return_val_if_fail(base_action != NULL, NULL);
	g_return_val_if_fail(g_variant_is_object_path(action_path), NULL);
	g_return_val_if_fail(g_variant_is_object_path(model_path), NULL);

	HudClientParam * param = g_object_new(HUD_CLIENT_TYPE_PARAM, NULL);

	param->priv->dbus_address = g_strdup(dbus_address);
	param->priv->base_action = g_strdup(base_action);
	param->priv->action_path = g_strdup(action_path);
	param->priv->model_path = g_strdup(model_path);
	param->priv->model_section = model_section;

	return param;
}


