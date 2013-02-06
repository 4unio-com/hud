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

	GMenuModel * model;
	GActionGroup * actions;
};

#define HUD_CLIENT_PARAM_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), HUD_CLIENT_TYPE_PARAM, HudClientParamPrivate))

static void hud_client_param_class_init (HudClientParamClass *klass);
static void hud_client_param_init       (HudClientParam *self);
static void hud_client_param_dispose    (GObject *object);
static void hud_client_param_finalize   (GObject *object);
static void action_write_state          (HudClientParam *  param,
                                         const gchar *     action);

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

	action_write_state(param, "end");

	g_clear_object(&param->priv->model);
	g_clear_object(&param->priv->actions);
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

/* Writes to the base action a particular state */
static void
action_write_state (HudClientParam * param, const gchar * action)
{
	/* This shows that they're not interested in these events,
	   which is fine, perhaps a bit lonely, but fine. */
	if (param->priv->base_action == NULL || param->priv->base_action[0] == '\0') {
		return;
	}

	g_return_if_fail(g_action_group_has_action(param->priv->actions, param->priv->base_action));

	const GVariantType * type = g_action_group_get_action_parameter_type(param->priv->actions, param->priv->base_action);

	GVariant * action_param = NULL;

	if (g_variant_type_equal(type, G_VARIANT_TYPE_STRING)) {
		action_param = g_variant_new_string(action);
	} else if (g_variant_type_equal(type, G_VARIANT_TYPE("(ssav)"))) {
		GVariantBuilder tuple;

		g_variant_builder_init(&tuple, G_VARIANT_TYPE_TUPLE);
		g_variant_builder_add_value(&tuple, g_variant_new_string(action));
		g_variant_builder_add_value(&tuple, g_variant_new_string(""));
		g_variant_builder_add_value(&tuple, g_variant_new_array(G_VARIANT_TYPE_VARIANT, NULL, 0));

		action_param = g_variant_builder_end(&tuple);
	}

	if (action_param == NULL) {
		gchar * typestring = g_variant_type_dup_string(type);
		g_warning("Unable to signal to a action with the parameter type of: %s", typestring);
		g_free(typestring);
		return;
	}

	g_action_group_activate_action(param->priv->actions, param->priv->base_action, action_param);

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

	g_warn_if_fail(model_section == 1);
	GDBusMenuModel * base_model = g_dbus_menu_model_get(param->priv->session, param->priv->dbus_address, param->priv->model_path);
	param->priv->model = g_menu_model_get_item_link(G_MENU_MODEL(base_model), 0, G_MENU_LINK_SUBMENU);
	g_object_unref(base_model);

	GDBusActionGroup * dbus_ag = g_dbus_action_group_get(param->priv->session, param->priv->dbus_address, param->priv->action_path);
	param->priv->actions = G_ACTION_GROUP(dbus_ag);

	action_write_state(param, "start");

	return param;
}

/**
 * hud_client_param_get_actions:
 * @param: The #HudClientParam to query
 *
 * The object path to the actions
 *
 * Return value: (transfer none): A #GActionGroup that has the actions in it
 */
GActionGroup *
hud_client_param_get_actions (HudClientParam * param)
{
	g_return_val_if_fail(HUD_CLIENT_IS_PARAM(param), NULL);

	return param->priv->actions;
}

/**
 * hud_client_param_get_model:
 * @param: The #HudClientParam to query
 *
 * The object path to the model
 *
 * Return value: (transfer none): The menu model of the pane
 */
GMenuModel *
hud_client_param_get_model (HudClientParam * param)
{
	g_return_val_if_fail(HUD_CLIENT_IS_PARAM(param), NULL);

	return param->priv->model;
}

/**
 * hud_client_param_send_reset:
 * @param: The #HudClientParam to query
 *
 * Send the command to the application to reset the values
 * of the actions in the pane.
 */
void
hud_client_param_send_reset (HudClientParam * param)
{
	g_return_if_fail(HUD_CLIENT_IS_PARAM(param));

	action_write_state(param, "reset");
	return;
}

/**
 * hud_client_param_send_cancel:
 * @param: The #HudClientParam to query
 *
 * Send the command to the application to cancel the values
 * of the actions in the panel and expect it to close soon.
 */
void
hud_client_param_send_cancel (HudClientParam * param)
{
	g_return_if_fail(HUD_CLIENT_IS_PARAM(param));

	action_write_state(param, "cancel");
	return;
}
