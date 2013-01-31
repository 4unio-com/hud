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

struct _HudClientParamPrivate {
	int dummy;
};

#define HUD_CLIENT_PARAM_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), HUD_CLIENT_PARAM_TYPE, HudClientParamPrivate))

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
	return;
}

static void
hud_client_param_dispose (GObject *object)
{

	G_OBJECT_CLASS (hud_client_param_parent_class)->dispose (object);
	return;
}

static void
hud_client_param_finalize (GObject *object)
{

	G_OBJECT_CLASS (hud_client_param_parent_class)->finalize (object);
	return;
}

/**
 * hud_client_param_new:
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
hud_client_param_new (const gchar * base_action, const gchar * action_path, const gchar * model_path, gint model_section)
{



	return NULL;
}


