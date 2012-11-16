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

#include "hudactionpublisher.h"

#include <gio/gio.h>

typedef struct
{
  GObject parent_instance;

  HudActionPublisher *publisher;
} HudAux;

typedef GObjectClass HudActionPublisherClass;

struct _HudActionPublisher
{
  GObject parent_instance;

  GSimpleActionGroup *actions;
  GSequence *description_sequence;
  GHashTable *description_table;
  GSequenceIter *eos;
  HudAux *aux;
};

enum
{
  SIGNAL_BEFORE_EMIT,
  SIGNAL_AFTER_EMIT,
  N_SIGNALS
};

guint hud_action_publisher_signals[N_SIGNALS];

G_DEFINE_TYPE (HudActionPublisher, hud_action_publisher, G_TYPE_OBJECT)

static void
hud_action_publisher_finalize (GObject *object)
{
  g_error ("g_object_unref() called on internally-owned ref of HudActionPublisher");
}

static void
hud_action_publisher_init (HudActionPublisher *publisher)
{
}

static void
hud_action_publisher_class_init (HudActionPublisherClass *class)
{
  class->finalize = hud_action_publisher_finalize;

  hud_action_publisher_signals[SIGNAL_BEFORE_EMIT] = g_signal_new ("before-emit", HUD_TYPE_ACTION_PUBLISHER,
                                                                   G_SIGNAL_RUN_LAST, 0, NULL, NULL,
                                                                   g_cclosure_marshal_VOID__VARIANT,
                                                                   G_TYPE_NONE, 1, G_TYPE_VARIANT);
  hud_action_publisher_signals[SIGNAL_AFTER_EMIT] = g_signal_new ("after-emit", HUD_TYPE_ACTION_PUBLISHER,
                                                                  G_SIGNAL_RUN_LAST, 0, NULL, NULL,
                                                                  g_cclosure_marshal_VOID__VARIANT,
                                                                  G_TYPE_NONE, 1, G_TYPE_VARIANT);
}

HudActionPublisher *
hud_action_publisher_get (void)
{
  static HudActionPublisher *publisher;

  if (!publisher)
    publisher = g_object_new (HUD_TYPE_ACTION_PUBLISHER, NULL);

  return publisher;
}

static gboolean
variant_equal0 (GVariant *a,
                GVariant *b)
{
  if (a == b)
    return TRUE;

  if (!a || !b)
    return FALSE;

  return g_variant_equal (a, b);
}

void
hud_action_publisher_add_description (HudActionPublisher   *publisher,
                                      HudActionDescription *description)
{
  GSequenceIter *iter;
  const gchar *name;
  GVariant *target;

  name = hud_action_description_get_action_name (description);
  target = hud_action_description_get_action_target (description);

  iter = g_hash_table_lookup (publisher->description_table, name);

  if (iter == NULL)
    {
      /* We do not have any actions with this name.
       *
       * Add the header for this action name.
       */
      iter = g_sequence_insert_before (publisher->eos, NULL);
      g_hash_table_insert (publisher->description_table, g_strdup (name), iter);

      /* Add the actual description */
      g_sequence_insert_before (publisher->eos, g_object_ref (description));

      /* Signal that we added two items */
      g_menu_model_items_changed (G_MENU_MODEL (publisher->aux), g_sequence_iter_get_position (iter), 0, 2);
    }
  else
    {
      HudActionDescription *item;

      while ((item = g_sequence_get (iter)))
        if (variant_equal0 (hud_action_description_get_action_target (item), target))
          break;

      if (item != NULL)
        {
          /* Replacing an existing item with the same action name/target */
          g_sequence_set (iter, g_object_ref (description));

          /* A replace is 1 remove and 1 add */
          g_menu_model_items_changed (G_MENU_MODEL (publisher->aux), g_sequence_iter_get_position (iter), 1, 1);
        }
      else
        {
          /* Adding a new item (with unique target value) */
          iter = g_sequence_insert_before (iter, g_object_ref (description));

          /* Just one add this time */
          g_menu_model_items_changed (G_MENU_MODEL (publisher->aux), g_sequence_iter_get_position (iter), 0, 1);
        }
    }
}

void
hud_action_publisher_remove_description (HudActionPublisher *publisher,
                                         const gchar        *action_name,
                                         GVariant           *action_target)
{
  HudActionDescription *item;
  GSequenceIter *header;
  GSequenceIter *start;
  GSequenceIter *iter;
  GSequenceIter *next;

  header = g_hash_table_lookup (publisher->description_table, action_name);

  /* No descriptions with this action name? */
  if (!header)
    return;

  /* Don't search the header itself... */
  start = iter = g_sequence_iter_next (header);
  while ((item = g_sequence_get (iter)))
    {
      if (variant_equal0 (hud_action_description_get_action_target (item), action_target))
        break;

      iter = g_sequence_iter_next (iter);
    }

  /* No description with this action target? */
  if (item == NULL)
    return;

  /* Okay.  We found our item (and iter).
   *
   * Is it the only one for this name (ie: is it the start one and there
   * is no following one)?
   */
  next = g_sequence_iter_next (iter);
  if (iter == start && g_sequence_get (next) == NULL)
    {
      /* It was the only one.  Remove it and the header. */
      g_hash_table_remove (publisher->description_table, action_name);
      g_sequence_remove_range (start, next);

      /* Signal both removes */
      g_menu_model_items_changed (G_MENU_MODEL (publisher->aux), g_sequence_iter_get_position (next), 2, 0);
    }
  else
    {
      /* There were others.  Only do one remove. */
      g_sequence_remove (iter);
      g_menu_model_items_changed (G_MENU_MODEL (publisher->aux), g_sequence_iter_get_position (next), 1, 0);
    }
}

void
hud_action_publisher_remove_descriptions (HudActionPublisher *publisher,
                                          const gchar        *action_name)
{
  GSequenceIter *header;
  GSequenceIter *end;
  gint p, r;

  header = g_hash_table_lookup (publisher->description_table, action_name);
  if (!header)
    return;

  g_hash_table_remove (publisher->description_table, action_name);

  end = g_sequence_iter_next (header);
  while (g_sequence_get (end))
    end = g_sequence_iter_next (end);

  p = g_sequence_iter_get_position (header);
  r = g_sequence_iter_get_position (end) - p;
  g_sequence_remove_range (header, end);

  g_menu_model_items_changed (G_MENU_MODEL (publisher->aux), p, r, 0);
}

void
hud_action_publisher_add_descriptions_from_file (HudActionPublisher *publisher,
                                                 const gchar        *filename)
{
}
