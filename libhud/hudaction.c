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
  gchar *name;
  gboolean enabled;

  const HudActionEntry *entry;
  gpointer user_data;
};

enum
{
  PROP_0,
  PROP_ENABLED,
  PROP_NAME,
  PROP_PARAMETER_TYPE,
  PROP_STATE,
  PROP_STATE_TYPE,
  PROP_IS_ACTIVE,
  PROP_OPERATION
};

enum
{
  SIGNAL_CREATE_OPERATION,
  SIGNAL_OPENED,
  SIGNAL_CLOSED,
  N_SIGNALS
};

guint hud_action_signals[N_SIGNALS];

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
  return G_VARIANT_TYPE ("(ssav)");
}

static const GVariantType *
hud_action_get_state_type (GAction *g_action)
{
  return G_VARIANT_TYPE ("aa{s(bgav)}");
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

static void
hud_action_operation_updated (HudAction *action)
{
  g_object_notify (G_OBJECT (action), "state");
}

static GVariant *
hud_action_get_state (GAction *g_action)
{
  HudAction *action = HUD_ACTION (g_action);
  GVariantBuilder builder;

  g_variant_builder_init (&builder, G_VARIANT_TYPE ("aa{s(bgav)}"));

  if (action->priv->operation)
    {
      GActionGroup *group;
      gchar **actions;
      gint i;

      group = G_ACTION_GROUP (action->priv->operation->priv->group);

      g_variant_builder_open (&builder, G_VARIANT_TYPE ("a{s(bgav)}"));

      actions = g_action_group_list_actions (group);
      for (i = 0; actions[i]; i++)
        {
          const GVariantType *parameter_type;
          gchar *typestring;
          GVariant *state;

          parameter_type = g_action_group_get_action_parameter_type (group, actions[i]);
          typestring = g_variant_type_dup_string (parameter_type);
          state = g_action_group_get_action_state (group, actions[i]);

          g_variant_builder_add (&builder, "{s(bgav)}", actions[i],
                                 g_action_group_get_action_enabled (group, actions[i]), typestring,
                                 g_variant_new_array (G_VARIANT_TYPE_VARIANT, &state, state ? 1 : 0));

          g_free (typestring);
        }

      g_variant_builder_close (&builder);
      g_strfreev (actions);
    }

  return g_variant_ref_sink (g_variant_builder_end (&builder));
}

static void
hud_action_change_state (GAction  *g_action,
                         GVariant *value)
{
}

static void
hud_action_activate (GAction  *g_action,
                     GVariant *value)
{
  HudAction *action = HUD_ACTION (g_action);
  const gchar *op;
  const gchar *name;
  GVariantIter *iter;
  GVariant *args;

  g_variant_get (value, "(&s&sav)", &op, &name, &iter);
  if (!g_variant_iter_next (iter, "v", &args))
    args = NULL;
  g_variant_iter_free (iter);

  if (g_str_equal (op, "start"))
    {
      if (action->priv->operation == NULL)
        g_signal_emit (action, hud_action_signals[SIGNAL_CREATE_OPERATION], 0, args);
    }

  else if (g_str_equal (op, "change-state"))
    {
      if (action->priv->operation != NULL)
        g_action_group_change_action_state (G_ACTION_GROUP (action->priv->operation->priv->group), name, args);
    }

  else if (g_str_equal (op, "activate"))
    {
      if (action->priv->operation != NULL)
        g_action_group_activate_action (G_ACTION_GROUP (action->priv->operation->priv->group), name, args);
    }

  else if (g_str_equal (op, "end"))
    {
      hud_action_set_operation (action, NULL);
    }

  if (args)
    g_variant_unref (args);
}

static void
hud_action_real_create_operation (HudAction *action,
                                  GVariant  *parameters)
{
  if (action->priv->operation == NULL)
    {
      HudOperation *operation;

      operation = hud_operation_new ();
      hud_operation_setup (operation, parameters);
      hud_action_set_operation (action, operation);
      g_object_unref (operation);
    }
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

    case PROP_IS_ACTIVE:
      g_value_set_boolean (value, action->priv->operation != NULL);
      break;

    case PROP_OPERATION:
      g_value_set_object (value, action->priv->operation);
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

    case PROP_OPERATION:
      hud_action_set_operation (action, g_value_get_object (value));
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
                                   g_param_spec_string ("name", "Action Name", "The name used to invoke the action",
                                                        NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
                                                        G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (object_class, PROP_ENABLED,
                                   g_param_spec_boolean ("enabled", "Enabled", "If the action can be activated", TRUE,
                                                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT |
                                                        G_PARAM_STATIC_STRINGS));

  /* is-active and operation are not GAction properties at all... */
  g_object_class_install_property (object_class, PROP_IS_ACTIVE,
                                   g_param_spec_boolean ("is-active", "Is active",
                                                         "If operation is currently in progress",
                                                         FALSE, G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_OPERATION,
                                   g_param_spec_object ("operation", "Operation",
                                                        "The operation object, if active, else, NULL",
                                                        HUD_TYPE_OPERATION,
                                                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  hud_action_signals[SIGNAL_CREATE_OPERATION] = g_signal_new ("create-operation", HUD_TYPE_ACTION, G_SIGNAL_RUN_LAST,
                                                              G_STRUCT_OFFSET (HudActionClass, create_operation),
                                                              NULL, NULL, g_cclosure_marshal_VOID__VARIANT,
                                                              G_TYPE_NONE, 1, G_TYPE_VARIANT);
  hud_action_signals[SIGNAL_OPENED] = g_signal_new ("opened", HUD_TYPE_ACTION, G_SIGNAL_RUN_LAST,
                                                    G_STRUCT_OFFSET (HudActionClass, opened),
                                                    NULL, NULL, g_cclosure_marshal_VOID__OBJECT,
                                                    G_TYPE_NONE, 1, G_TYPE_OBJECT);
  hud_action_signals[SIGNAL_CLOSED] = g_signal_new ("closed", HUD_TYPE_ACTION, G_SIGNAL_RUN_LAST,
                                                    G_STRUCT_OFFSET (HudActionClass, closed),
                                                    NULL, NULL, g_cclosure_marshal_VOID__VOID,
                                                    G_TYPE_NONE, 0);
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

void
hud_action_set_enabled (HudAction *action,
                        gboolean   enabled)
{
  enabled = !!enabled;

  if (action->priv->enabled != enabled)
    {
      action->priv->enabled = enabled;
      g_object_notify (G_OBJECT (action), "enabled");
    }
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
      g_signal_handlers_disconnect_by_func (action->priv->operation,
                                            hud_action_operation_updated,
                                            action);
      hud_operation_ended (action->priv->operation);
      g_signal_emit (action, hud_action_signals[SIGNAL_CLOSED], 0);
      g_object_unref (action->priv->operation);
      action->priv->operation = NULL;
    }

  if (operation)
    {
      action->priv->operation = g_object_ref (operation);
      g_signal_connect_swapped (action->priv->operation->priv->group, "action-added",
                                G_CALLBACK (hud_action_operation_updated), action);
      g_signal_connect_swapped (action->priv->operation->priv->group, "action-removed",
                                G_CALLBACK (hud_action_operation_updated), action);
      g_signal_connect_swapped (action->priv->operation->priv->group, "action-enabled-changed",
                                G_CALLBACK (hud_action_operation_updated), action);
      g_signal_connect_swapped (action->priv->operation->priv->group, "action-state-changed",
                                G_CALLBACK (hud_action_operation_updated), action);
      g_signal_emit (action, hud_action_signals[SIGNAL_OPENED], 0, action->priv->operation);
      hud_operation_started (action->priv->operation);
    }

  g_object_notify (G_OBJECT (action), "state");
}

void
hud_action_entries_install (GActionMap           *map,
                            const HudActionEntry *entries,
                            guint                 n_entries,
                            gpointer              user_data)
{
  guint i;

  for (i = 0; i < n_entries; i++)
    {
      const HudActionEntry *entry = &entries[i];
      HudAction *action;

      action = hud_action_new (entry->name);
      action->priv->entry = entry;
      action->priv->user_data = user_data;

      g_action_map_add_action (map, G_ACTION (action));
    }
}
