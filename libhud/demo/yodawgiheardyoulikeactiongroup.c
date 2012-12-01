#include "yodawgiheardyoulikeactiongroup.h"

typedef GObjectClass YoDawgIHeardYouLikeActionGroupClass;

typedef struct
{
  GObject parent_instance;

  GActionGroup *group;
  gchar *action_name;

  /* So we can diff state updates and emit the proper signals */
  GHashTable *actions;
} YoDawgIHeardYouLikeActionGroup;

typedef struct
{
  gboolean enabled;
  GVariantType *parameter_type;
  GVariant *state;
} Action;

static void
action_free (gpointer data)
{
  Action *action = data;

  if (action->parameter_type)
    g_variant_type_free (action->parameter_type);

  if (action->state)
    g_variant_unref (action->state);

  g_slice_free (Action, action);
}

static void yo_dawg_i_heard_you_like_action_group_iface_init (GActionGroupInterface *iface);
G_DEFINE_TYPE_WITH_CODE (YoDawgIHeardYouLikeActionGroup, yo_dawg_i_heard_you_like_action_group, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (G_TYPE_ACTION_GROUP, yo_dawg_i_heard_you_like_action_group_iface_init))

static gchar **
yo_dawg_i_heard_you_like_action_group_list_actions (GActionGroup *g_group)
{
  YoDawgIHeardYouLikeActionGroup *group = (YoDawgIHeardYouLikeActionGroup *) g_group;
  GHashTableIter iter;
  gint n, i = 0;
  gchar **keys;
  gpointer key;

  n = g_hash_table_size (group->actions);
  keys = g_new (gchar *, n + 1);

  g_hash_table_iter_init (&iter, group->actions);
  while (g_hash_table_iter_next (&iter, &key, NULL))
    keys[i++] = g_strdup (key);
  g_assert_cmpint (i, ==, n);
  keys[n] = NULL;

  return keys;
}

static gboolean
yo_dawg_i_heard_you_like_action_group_query_action (GActionGroup        *g_group,
                                                    const gchar         *action_name,
                                                    gboolean            *enabled,
                                                    const GVariantType **parameter_type,
                                                    const GVariantType **state_type,
                                                    GVariant           **state_hint,
                                                    GVariant           **state)
{
  YoDawgIHeardYouLikeActionGroup *group = (YoDawgIHeardYouLikeActionGroup *) g_group;
  Action *action;

  action = g_hash_table_lookup (group->actions, action_name);

  if (action == NULL)
    return FALSE;

  if (enabled)
    *enabled = action->enabled;

  if (parameter_type)
    *parameter_type = action->parameter_type;

  if (state_type)
    *state_type = action->state ? g_variant_get_type (action->state) : NULL;

  if (state_hint)
    *state_hint = NULL;

  if (state)
    *state = action->state ? g_variant_ref (action->state) : NULL;

  return TRUE;
}

static GVariant *
fake_mv (GVariant *value)
{
  if (value)
    value = g_variant_new_variant (value);

  return g_variant_new_array (G_VARIANT_TYPE_VARIANT, &value, value != NULL);
}

static void
unfake_mv (GVariant **value)
{
  GVariant *child;

  if (g_variant_n_children (*value))
    g_variant_get_child (*value, 0, "v", &child);
  else
    child = NULL;

  g_variant_unref (*value);
  *value = child;
}

static void
yo_dawg_i_heard_you_like_action_group_change_state (GActionGroup *g_group,
                                                    const gchar  *action_name,
                                                    GVariant     *value)
{
  YoDawgIHeardYouLikeActionGroup *group = (YoDawgIHeardYouLikeActionGroup *) g_group;

  g_action_group_activate_action (group->group, group->action_name,
                                  g_variant_new ("(ss@av)", "change-state", action_name, fake_mv (value)));
}

static void
yo_dawg_i_heard_you_like_action_group_activate (GActionGroup *g_group,
                                                const gchar  *action_name,
                                                GVariant     *parameter)
{
  YoDawgIHeardYouLikeActionGroup *group = (YoDawgIHeardYouLikeActionGroup *) g_group;

  g_action_group_activate_action (group->group, group->action_name,
                                  g_variant_new ("(ss@av)", "activate", action_name, fake_mv (parameter)));
}

static void
yo_dawg_i_heard_you_like_action_group_state_changed (GActionGroup *parent,
                                                     const gchar  *action_name,
                                                     GVariant     *state,
                                                     gpointer      user_data)
{
  YoDawgIHeardYouLikeActionGroup *group = user_data;
  GHashTable *to_remove;

  {
    GHashTableIter iter;
    gpointer key;

    /* We need to take a copy of the key because we'll want to emit a
     * removed signal on ourselves after removing it.
     */
    to_remove = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
    g_hash_table_iter_init (&iter, group->actions);
    while (g_hash_table_iter_next (&iter, &key, NULL))
      g_hash_table_add (to_remove, g_strdup (key));
  }

  if (g_variant_n_children (state))
    {
      const gchar *name;
      gboolean enabled;
      GVariant *action_state;
      const gchar *parameter_typestr;
      GVariantIter *iter;

      g_variant_get_child (state, 0, "a{s(bgav)}", &iter);
      while (g_variant_iter_loop (iter, "{&s(b&g@av)}", &name, &enabled, &parameter_typestr, &action_state))
        {
          const GVariantType *parameter_type;
          Action *action;

          unfake_mv (&action_state);

          if (parameter_typestr[0])
            {
              if (!g_variant_type_string_is_valid (parameter_typestr))
                {
                  g_warning ("Subaction '%s' in action '%s' has invalid typestring %s.  Ignoring.",
                             name, group->action_name, parameter_typestr);
                  parameter_typestr = "";
                }

              parameter_type = G_VARIANT_TYPE (parameter_typestr);
            }
          else
            parameter_type = NULL;

          /* We will not need to remove this action */
          g_hash_table_remove (to_remove, name);


          action = g_hash_table_lookup (group->actions, name);
          if (action == NULL)
            {
              /* We are adding a new action */
              action = g_slice_new (Action);
              action->enabled = enabled;
              action->parameter_type = parameter_type ? g_variant_type_copy (parameter_type) : NULL;
              action->state = action_state ? g_variant_ref (action_state) : NULL;

              g_hash_table_insert (group->actions, g_strdup (name), action);

              g_action_group_action_added (G_ACTION_GROUP (group), name);
            }
          else
            {
              /* We are (maybe) updating an existing action */
              if (action->enabled != enabled)
                {
                  action->enabled = enabled;
                  g_action_group_action_enabled_changed (G_ACTION_GROUP (group), name, enabled);
                }

              if (action->parameter_type != parameter_type &&
                  (!action->parameter_type || !parameter_type ||
                   !g_variant_type_equal (action->parameter_type, parameter_type)))
                {
                  gchar *old, *new;

                  old = action->parameter_type ? g_variant_type_dup_string (action->parameter_type) : g_strdup ("nil");
                  new = parameter_type ? g_variant_type_dup_string (parameter_type) : g_strdup ("nil");

                  g_warning ("Subaction '%s' in action '%s' illegally changed parameter type from %s to %s",
                             name, group->action_name, old, new);

                  g_free (old);
                  g_free (new);
                }

              if (action->state != action_state)
                {
                  if (!action->state || !action_state || !g_variant_type_equal (g_variant_get_type (action->state),
                                                                                g_variant_get_type (action_state)))
                    {
                      const gchar *old, *new;

                      old = action->state ? g_variant_get_type_string (action->state) : "nil";
                      new = action_state ? g_variant_get_type_string (action_state) : "nil";

                      g_warning ("Subaction '%s' in action '%s' illegally changed state type from %s to %s",
                                 name, group->action_name, old, new);
                    }
                  else
                    {
                      if (!g_variant_equal (action->state, state))
                        {
                          g_variant_unref (action->state);
                          action->state = g_variant_ref (action_state);

                          g_action_group_action_state_changed (G_ACTION_GROUP (group), name, action_state);
                        }
                    }
                }

              if (action_state)
                g_variant_unref (action_state);
            }
        }

      g_variant_iter_free (iter);
    }

    {
      GHashTableIter iter;
      gpointer key;

      g_hash_table_iter_init (&iter, to_remove);
      while (g_hash_table_iter_next (&iter, &key, NULL))
        {
          g_hash_table_remove (group->actions, key);
          g_action_group_action_removed (G_ACTION_GROUP (group), key);
          g_hash_table_iter_remove (&iter);
        }

      g_hash_table_unref (to_remove);
    }
}

static void
yo_dawg_i_heard_you_like_action_group_finalize (GObject *object)
{
  YoDawgIHeardYouLikeActionGroup *group = (YoDawgIHeardYouLikeActionGroup *) object;

  g_action_group_activate_action (group->group, group->action_name,
                                  g_variant_new ("(ss@av)", "end", "", fake_mv (NULL)));

  g_signal_handlers_disconnect_by_func (group->group, yo_dawg_i_heard_you_like_action_group_state_changed, group);
  g_hash_table_unref (group->actions);
  g_object_unref (group->group);
  g_free (group->action_name);

  G_OBJECT_CLASS (yo_dawg_i_heard_you_like_action_group_parent_class)
    ->finalize (object);
}

static void
yo_dawg_i_heard_you_like_action_group_init (YoDawgIHeardYouLikeActionGroup *group)
{
  group->actions = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, action_free);
}

static void
yo_dawg_i_heard_you_like_action_group_iface_init (GActionGroupInterface *iface)
{
  iface->list_actions = yo_dawg_i_heard_you_like_action_group_list_actions;
  iface->query_action = yo_dawg_i_heard_you_like_action_group_query_action;
  iface->change_action_state = yo_dawg_i_heard_you_like_action_group_change_state;
  iface->activate_action = yo_dawg_i_heard_you_like_action_group_activate;
}

static void
yo_dawg_i_heard_you_like_action_group_class_init (YoDawgIHeardYouLikeActionGroupClass *class)
{
  class->finalize = yo_dawg_i_heard_you_like_action_group_finalize;
}

GActionGroup *
yo_dawg_i_heard_you_like_action_group_start (GActionGroup *parent,
                                             const gchar  *action_name,
                                             GVariant     *setup_data)
{
  YoDawgIHeardYouLikeActionGroup *group;
  GVariant *state;
  gchar *detailed;

  g_return_val_if_fail (G_IS_ACTION_GROUP (parent), NULL);
  g_return_val_if_fail (action_name != NULL, NULL);
  g_return_val_if_fail (setup_data == NULL || g_variant_is_of_type (setup_data, G_VARIANT_TYPE_VARDICT), NULL);

  /* XXX: This code implicitly assumes that the action exists, is
   * enabled, and will always exist and be enabled for the duration of
   * the operation.
   *
   * That's probably true but there may be races or other weird
   * situations...
   */
  state = g_action_group_get_action_state (parent, action_name);
  if (!state || !g_variant_is_of_type (state, G_VARIANT_TYPE ("aa{s(bgav)}")))
    {
      g_warning ("Attempting to create YoDawgIHeardYouLikeActionGroup for action '%s' "
                 "of type '%s', which is not the correct state type", action_name,
                 state ? g_variant_get_type_string (state) : "(nil)");
      return NULL;
    }

  group = g_object_new (yo_dawg_i_heard_you_like_action_group_get_type (), NULL);
  group->group = g_object_ref (parent);
  group->action_name = g_strdup (action_name);

  g_action_group_activate_action (parent, action_name, g_variant_new ("(ss@av)", "start", "", fake_mv (setup_data)));

  detailed = g_strdup_printf ("action-state-changed::%s", action_name);
  g_signal_connect (parent, detailed, G_CALLBACK (yo_dawg_i_heard_you_like_action_group_state_changed), group);
  g_free (detailed);

  yo_dawg_i_heard_you_like_action_group_state_changed (parent, action_name, state, group);
  g_variant_unref (state);

  return (GActionGroup *) group;
}
