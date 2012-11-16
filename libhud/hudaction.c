/*
 * Copyright © 2012 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of either or both of the following licences:
 *
 * 1) the GNU Lesser General Public License version 3, as published by
 * the Free Software Foundation; and/or
 * 2) the GNU Lesser General Public License version 2.1, as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the applicable version of the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of both the GNU Lesser General Public
 * License version 3 and version 2.1 along with this program.  If not,
 * see <http://www.gnu.org/licenses/>
 *
 * Author: Ryan Lortie <desrt@desrt.ca>
 */

#include "hudaction.h"

#include "hudoperation-private.h"

struct _HudActionPrivate {
  HudOperation *operation;
  gboolean enabled;
  gchar *name;
};

enum
{
  PROP_0,
  PROP_ENABLED,
  PROP_NAME,
  PROP_PARAMETER_TYPE,
  PROP_STATE,
  PROP_STATE_TYPE
};

guint hud_action_signal_create_operation;

static void hud_action_iface_init (GActionInterface *iface);

G_DEFINE_TYPE_WITH_CODE (HudAction, hud_action, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (G_TYPE_ACTION, hud_action_iface_init))

static const gchar *
hud_action_get_name (GAction *g_action)
{
  HudAction *action = HUD_ACTION (g_action);

  return action->priv->name;
}

static const GVariantType *
hud_action_get_parameter_type (GAction *g_action)
{
  return G_VARIANT_TYPE ("(sa{sv})");
}

static const GVariantType *
hud_action_get_state_type (GAction *g_action)
{
  return G_VARIANT_TYPE_BOOLEAN;
}

static GVariant *
hud_action_get_state_hint (GAction *g_action)
{
  return NULL;
}

static gboolean
hud_action_get_enabled (GAction *g_action)
{
  HudAction *action = HUD_ACTION (g_action);

  return action->priv->enabled;
}

static GVariant *
hud_action_get_state (GAction *g_action)
{
  HudAction *action = HUD_ACTION (g_action);

  return g_variant_ref_sink (g_variant_new_boolean (hud_action_get_is_active (action)));
}

static void
hud_action_change_state (GAction  *g_action,
                         GVariant *value)
{
  HudAction *action = HUD_ACTION (g_action);

  if (!g_variant_get_boolean (value))
    hud_action_set_operation (action, NULL);
}

static void
hud_action_activate (GAction  *g_action,
                     GVariant *value)
{
  HudAction *action = HUD_ACTION (g_action);
  const gchar *op;
  GVariant *args;

  g_variant_get (value, "(&s@a{sv})", &op, &args);

  if (g_str_equal (op, "start"))
    {
      if (action->priv->operation == NULL)
        g_signal_emit (action, hud_action_signal_create_operation, 0, args);
    }

  else if (g_str_equal (op, "update"))
    {
      if (action->priv->operation != NULL)
        hud_operation_update (action->priv->operation, args);
    }

  else if (g_str_equal (op, "response"))
    {
      if (action->priv->operation != NULL)
        hud_operation_response (action->priv->operation, args);
    }

  else if (g_str_equal (op, "end"))
    hud_action_set_operation (action, NULL);

  g_variant_unref (args);
}

static void
hud_action_real_create_operation (HudAction *action,
                                  GVariant  *parameters)
{
  HudOperation *operation;

  if (action->priv->operation)
    operation = g_object_ref (action->priv->operation);
  else
    operation = hud_operation_new ();

  hud_operation_setup (operation, parameters);

  hud_action_set_operation (action, operation);

  g_object_unref (operation);
}

static void
hud_action_get_property (GObject *object, guint prop_id,
                         GValue *value, GParamSpec *pspec)
{
  HudAction *action = HUD_ACTION (object);

  switch (prop_id)
    {
    case PROP_ENABLED:
      g_value_set_boolean (value, action->priv->enabled);
      break;

    case PROP_NAME:
      g_value_set_string (value, action->priv->name);
      break;

    case PROP_PARAMETER_TYPE:
      g_value_set_boxed (value, G_VARIANT_TYPE ("(sa{sv})"));
      break;

    case PROP_STATE:
      g_value_take_variant (value, g_variant_new_boolean (hud_action_get_is_active (action)));
      break;

    case PROP_STATE_TYPE:
      g_value_set_boxed (value, G_VARIANT_TYPE_BOOLEAN);
      break;

    default:
      g_assert_not_reached ();
    }
}

static void
hud_action_set_property (GObject *object, guint prop_id,
                         const GValue *value, GParamSpec *pspec)
{
  HudAction *action = HUD_ACTION (object);

  switch (prop_id)
    {
    case PROP_ENABLED:
      hud_action_set_enabled (action, g_value_get_boolean (value));
      break;

    case PROP_NAME:
      g_assert (!action->priv->name);
      action->priv->name = g_value_dup_string (value);
      break;

    default:
      g_assert_not_reached ();
    }
}

static void
hud_action_finalize (GObject *object)
{
  HudAction *action = HUD_ACTION (object);

  g_clear_object (&action->priv->operation);
  g_free (action->priv->name);

  G_OBJECT_CLASS (hud_action_parent_class)
    ->finalize (object);
}

static void
hud_action_init (HudAction *action)
{
  action->priv = G_TYPE_INSTANCE_GET_PRIVATE (action, HUD_TYPE_ACTION, HudActionPrivate);
}

static void
hud_action_class_init (HudActionClass *class)
{
  GObjectClass *object_class = G_OBJECT_CLASS (class);

  class->create_operation = hud_action_real_create_operation;

  object_class->set_property = hud_action_set_property;
  object_class->get_property = hud_action_get_property;
  object_class->finalize = hud_action_finalize;

  g_type_class_add_private (class, sizeof (HudActionPrivate));

  /* These three properties stay read-only as per the GAction iface */
  g_object_class_override_property (object_class, PROP_PARAMETER_TYPE, "parameter-type");
  g_object_class_override_property (object_class, PROP_STATE, "state");
  g_object_class_override_property (object_class, PROP_STATE_TYPE, "state-type");

  /* Name becomes constructable and enabled becomes writable... */
  g_object_class_install_property (object_class, PROP_NAME,
                                   g_param_spec_string ("name", "Action Name", "The name used to invoke the action", NULL,
                                                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (object_class, PROP_ENABLED,
                                   g_param_spec_boolean ("enabled", "Enabled", "If the action can be activated", TRUE,
                                                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS));

  hud_action_signal_create_operation = g_signal_new ("create-operation", HUD_TYPE_ACTION, G_SIGNAL_RUN_LAST,
                                                     G_STRUCT_OFFSET (HudActionClass, create_operation), NULL, NULL,
                                                     g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1, G_TYPE_OBJECT);
}

static void
hud_action_iface_init (GActionInterface *iface)
{
  iface->get_name = hud_action_get_name;
  iface->get_parameter_type = hud_action_get_parameter_type;
  iface->get_state_type = hud_action_get_state_type;
  iface->get_state_hint = hud_action_get_state_hint;
  iface->get_enabled = hud_action_get_enabled;
  iface->get_state = hud_action_get_state;
  iface->change_state = hud_action_change_state;
  iface->activate = hud_action_activate;
}

HudAction *
hud_action_new (const gchar  *name)
{
  return g_object_new (HUD_TYPE_ACTION,
                       "name", name,
                       NULL);
}

gboolean
hud_action_get_is_active (HudAction *action)
{
  return action->priv->operation != NULL;
}

HudOperation *
hud_action_get_operation (HudAction *action)
{
  return action->priv->operation;
}

void
hud_action_set_operation (HudAction    *action,
                          HudOperation *operation)
{
  if (action->priv->operation == operation)
    return;

  if (action->priv->operation)
    {
      hud_operation_end (action->priv->operation);
      g_object_unref (action->priv->operation);
      action->priv->operation = NULL;
    }

  if (operation)
    action->priv->operation = g_object_ref (operation);

  g_object_notify (G_OBJECT (action), "state");
}
