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

#include "operation.h"

#include "operation-private.h"

G_DEFINE_TYPE (HudOperation, hud_operation, G_TYPE_SIMPLE_ACTION_GROUP)

enum
{
  SIGNAL_PREPARE,
  SIGNAL_STARTED,
  SIGNAL_CHANGED,
  SIGNAL_ENDED,
  N_SIGNALS
};

static guint hud_operation_signals[N_SIGNALS];

static void
hud_operation_finalize (GObject *object)
{
  HudOperation *operation = HUD_OPERATION (object);

  if (operation->priv->group)
    g_object_unref (operation->priv->group);

  G_OBJECT_CLASS (hud_operation_parent_class)
    ->finalize (object);
}

static void
hud_operation_init (HudOperation *operation)
{
  operation->priv = G_TYPE_INSTANCE_GET_PRIVATE (operation, HUD_TYPE_OPERATION, HudOperationPrivate);
  operation->priv->group = g_simple_action_group_new ();
}

static void
hud_operation_class_init (HudOperationClass *class)
{
  GObjectClass *object_class = G_OBJECT_CLASS (class);

  object_class->finalize = hud_operation_finalize;

  g_type_class_add_private (class, sizeof (HudOperationPrivate));

  hud_operation_signals[SIGNAL_STARTED] = g_signal_new ("started", HUD_TYPE_OPERATION, G_SIGNAL_RUN_FIRST,
                                                        G_STRUCT_OFFSET (HudOperationClass, started),
                                                        NULL, NULL, g_cclosure_marshal_VOID__VOID,
                                                        G_TYPE_NONE, 0);
  hud_operation_signals[SIGNAL_CHANGED] = g_signal_new ("changed", HUD_TYPE_OPERATION,
                                                        G_SIGNAL_RUN_FIRST | G_SIGNAL_DETAILED,
                                                        G_STRUCT_OFFSET (HudOperationClass, changed),
                                                        NULL, NULL, g_cclosure_marshal_VOID__STRING,
                                                        G_TYPE_NONE, 1, G_TYPE_STRING);
  hud_operation_signals[SIGNAL_ENDED] = g_signal_new ("ended", HUD_TYPE_OPERATION, G_SIGNAL_RUN_FIRST,
                                                      G_STRUCT_OFFSET (HudOperationClass, ended),
                                                      NULL, NULL, g_cclosure_marshal_VOID__VOID,
                                                      G_TYPE_NONE, 0);
}

HudOperation *
hud_operation_new (gpointer user_data)
{
  HudOperation *operation;

  operation = g_object_new (HUD_TYPE_OPERATION, NULL);
  operation->priv->user_data = user_data;

  return operation;
}

/**
 * hud_operation_setup:
 * @operation: a new #HudOperation
 * @parameters: the setup parameters
 *
 * Performs the initial setup for the operation -- assigning initial
 * values to all of the parameters.
 *
 * You don't need to call this function in the normal case.  It is only
 * needed If you override create_operation on #HudAction, in which case
 * you should call it on your #HudOperation subclass before adding it
 * back to the action using hud_action_set_operation().
 **/
void
hud_operation_setup (HudOperation *operation,
                     GVariant     *parameters)
{
  GVariantIter iter;
  const gchar *name;
  GVariant *value;

  g_variant_iter_init (&iter, parameters);
  while (g_variant_iter_loop (&iter, "{&sv}", &name, &value))
    g_action_group_change_action_state (G_ACTION_GROUP (operation->priv->group), name, value);
}

static void
hud_operation_action_state_changed (GActionGroup *action_group,
                                    const gchar  *action_name,
                                    GVariant     *state,
                                    gpointer      user_data)
{
  HudOperation *operation = user_data;

  g_assert (action_group == G_ACTION_GROUP (operation->priv->group));

  g_signal_emit (operation, hud_operation_signals[SIGNAL_CHANGED], g_quark_try_string (action_name), action_name);
}

/**
 * hud_operation_get_user_data:
 * @operation: a #HudOperation
 *
 * Requests the "user data" associated with a #HudOperation.
 *
 * If @operation is associated with a #HudAction created with a
 * #HudActionEntry (the normal case) then this is the user_data that was
 * passed as the last argument to hud_action_entries_install().
 **/
gpointer
hud_operation_get_user_data (HudOperation *operation)
{
  return operation->priv->user_data;
}

void
hud_operation_started (HudOperation *operation)
{
  g_signal_connect (operation->priv->group, "action-state-changed",
                    G_CALLBACK (hud_operation_action_state_changed), operation);
  g_signal_emit (operation, hud_operation_signals[SIGNAL_STARTED], 0);
}

void
hud_operation_ended (HudOperation *operation)
{
  g_signal_emit (operation, hud_operation_signals[SIGNAL_ENDED], 0);
  g_signal_handlers_disconnect_by_func (operation->priv->group, hud_operation_action_state_changed, operation);
}

/**
 * hud_operation_get_int:
 * @operation: a #HudOperation
 * @action_name: the name of an action in @operation
 *
 * This is a convenience API for querying the state of a int32-valued
 * action within @operation.
 *
 * It is an error to call this function if @action_name does not exist
 * or if its state is not an integer.
 *
 * Returns: the value of the state of the named action
 **/
gint
hud_operation_get_int (HudOperation *operation,
                       const gchar  *action_name)
{
  GVariant *variant;
  gint value;

  variant = g_action_group_get_action_state (G_ACTION_GROUP (operation->priv->group), action_name);
  value = g_variant_get_int32 (variant);
  g_variant_unref (variant);

  return value;
}

/**
 * hud_operation_get_uint:
 * @operation: a #HudOperation
 * @action_name: the name of an action in @operation
 *
 * This is a convenience API for querying the state of a uint32-valued
 * action within @operation.
 *
 * It is an error to call this function if @action_name does not exist
 * or if its state is not an unsigned integer.
 *
 * Returns: the value of the state of the named action
 **/
guint
hud_operation_get_uint (HudOperation *operation,
                        const gchar  *action_name)
{
  GVariant *variant;
  guint value;

  variant = g_action_group_get_action_state (G_ACTION_GROUP (operation->priv->group), action_name);
  value = g_variant_get_uint32 (variant);
  g_variant_unref (variant);

  return value;
}

/**
 * hud_operation_get_boolean:
 * @operation: a #HudOperation
 * @action_name: the name of an action in @operation
 *
 * This is a convenience API for querying the state of a boolean-valued
 * action within @operation.
 *
 * It is an error to call this function if @action_name does not exist
 * or if its state is not boolean-valued.
 *
 * Returns: the value of the state of the named action
 **/
gboolean
hud_operation_get_boolean (HudOperation *operation,
                           const gchar  *action_name)
{
  GVariant *variant;
  gboolean value;

  variant = g_action_group_get_action_state (G_ACTION_GROUP (operation->priv->group), action_name);
  value = g_variant_get_boolean (variant);
  g_variant_unref (variant);

  return value;
}

/**
 * hud_operation_get_double:
 * @operation: a #HudOperation
 * @action_name: the name of an action in @operation
 *
 * This is a convenience API for querying the state of a double-valued
 * action within @operation.
 *
 * It is an error to call this function if @action_name does not exist
 * or if its state is not double-valued.
 *
 * Returns: the value of the state of the named action
 **/
gdouble
hud_operation_get_double (HudOperation *operation,
                          const gchar  *action_name)
{
  GVariant *variant;
  gdouble value;

  variant = g_action_group_get_action_state (G_ACTION_GROUP (operation->priv->group), action_name);
  value = g_variant_get_double (variant);
  g_variant_unref (variant);

  return value;
}
