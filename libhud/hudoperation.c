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

#include "hudoperation.h"

#include "hudoperation-private.h"

G_DEFINE_TYPE (HudOperation, hud_operation, G_TYPE_OBJECT)

struct _HudOperationPrivate
{
  GHashTable *parameters;
};

enum
{
  SIGNAL_START,
  SIGNAL_UPDATE,
  SIGNAL_RESPONSE,
  SIGNAL_END,
  N_SIGNALS
};

static guint hud_operation_signals[N_SIGNALS];

static void
hud_operation_init (HudOperation *operation)
{
  operation->priv = G_TYPE_INSTANCE_GET_PRIVATE (operation, HUD_TYPE_OPERATION, HudOperationPrivate);
}

static void
hud_operation_class_init (HudOperationClass *class)
{
  g_type_class_add_private (class, sizeof (HudOperationPrivate));

  hud_operation_signals[SIGNAL_START] = g_signal_new ("start", HUD_TYPE_OPERATION, G_SIGNAL_RUN_FIRST,
                                                      G_STRUCT_OFFSET (HudOperationClass, start),
                                                      NULL, NULL, g_cclosure_marshal_VOID__VOID,
                                                      G_TYPE_NONE, 0);
  hud_operation_signals[SIGNAL_UPDATE] = g_signal_new ("update", HUD_TYPE_OPERATION,
                                                       G_SIGNAL_RUN_FIRST | G_SIGNAL_DETAILED,
                                                       G_STRUCT_OFFSET (HudOperationClass, update),
                                                       NULL, NULL, g_cclosure_marshal_VOID__STRING,
                                                       G_TYPE_NONE, 1, G_TYPE_STRING);
  hud_operation_signals[SIGNAL_RESPONSE] = g_signal_new ("response", HUD_TYPE_OPERATION,
                                                         G_SIGNAL_RUN_FIRST | G_SIGNAL_DETAILED,
                                                         G_STRUCT_OFFSET (HudOperationClass, response),
                                                         NULL, NULL, NULL,
                                                         G_TYPE_NONE, 1, G_TYPE_STRING, G_TYPE_VARIANT);
  hud_operation_signals[SIGNAL_END] = g_signal_new ("end", HUD_TYPE_OPERATION, G_SIGNAL_RUN_FIRST,
                                                    G_STRUCT_OFFSET (HudOperationClass, end),
                                                    NULL, NULL, g_cclosure_marshal_VOID__VOID,
                                                    G_TYPE_NONE, 0);
}

HudOperation *
hud_operation_new (void)
{
  return g_object_new (HUD_TYPE_OPERATION, NULL);
}

void
hud_operation_setup (HudOperation *operation,
                     GVariant     *parameters)
{
  GVariantIter iter;
  GVariant *value;
  gchar *key;

  g_return_if_fail (operation->priv->parameters == NULL);

  operation->priv->parameters = g_hash_table_new_full (g_str_hash, g_str_equal, g_free,
                                                       (GDestroyNotify) g_variant_unref);
  g_variant_iter_init (&iter, parameters);
  while (g_variant_iter_next (&iter, "{sv}", &key, &value))
    g_hash_table_insert (operation->priv->parameters, key, value);

  g_signal_emit (operation, hud_operation_signals[SIGNAL_START], 0);
}

void
hud_operation_update (HudOperation *operation,
                      GVariant     *parameters)
{
  GVariantIter iter;
  const gchar *key;
  GVariant *value;

  g_return_if_fail (operation->priv->parameters != NULL);

  g_variant_iter_init (&iter, parameters);
  while (g_variant_iter_loop (&iter, "{&sv}", &key, &value))
    {
      GVariant *old_value;

      old_value = g_hash_table_lookup (operation->priv->parameters, key);

      if (!old_value)
        {
          g_warning ("HUD sent update for parameter '%s' but it does not exist", key);
          continue;
        }

      if (g_variant_equal (value, old_value))
        continue;

      g_hash_table_insert (operation->priv->parameters, g_strdup (key), g_variant_ref (value));
      g_signal_emit (operation, hud_operation_signals[SIGNAL_UPDATE], g_quark_try_string (key), key);
    }
}

void
hud_operation_response (HudOperation *operation,
                        GVariant     *parameters)
{
  const gchar *response_id;
  GVariant *response_data;

  if (!g_variant_lookup (parameters, "response-id", "&s", &response_id))
    return;

  response_data = g_variant_lookup_value (parameters, "response-data", NULL);

  g_signal_emit (operation, hud_operation_signals[SIGNAL_RESPONSE],
                 g_quark_try_string (response_id), response_id, response_data);

  if (response_data)
    g_variant_unref (response_data);
}

void
hud_operation_end (HudOperation *operation)
{
  g_signal_emit (operation, hud_operation_signals[SIGNAL_END], 0);
}
