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
  publisher->description_sequence = g_sequence_new (g_object_unref);
  publisher->description_table = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
  publisher->eos = g_sequence_append (publisher->description_sequence, NULL);
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

#define g_menu_model_items_changed(x,...)

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

#include <string.h>

static gboolean
backport_g_action_parse_detailed_name (const gchar  *detailed_name,
                                       gchar       **action_name,
                                       GVariant    **target_value,
                                       GError      **error)
{
  const gchar *target;
  gsize target_len;
  gsize base_len;

  /* We decide which format we have based on which we see first between
   * '::' '(' and '\0'.
   */

  if (*detailed_name == '\0' || *detailed_name == ' ')
    goto bad_fmt;

  base_len = strcspn (detailed_name, ": ()");
  target = detailed_name + base_len;
  target_len = strlen (target);

  switch (target[0])
    {
    case ' ':
    case ')':
      goto bad_fmt;

    case ':':
      if (target[1] != ':')
        goto bad_fmt;

      *target_value = g_variant_ref_sink (g_variant_new_string (target + 2));
      break;

    case '(':
      {
        if (target[target_len - 1] != ')')
          goto bad_fmt;

        /* This is a bit tricky.  We want to allow strings like ('x') to
         * mean "the string X" while allowing strings like ('x', 'y') to
         * represent pairs and even allowing ('x',) to represent
         * one-tuples.
         *
         * Fortunately the two cases are completely disjoint; we can
         * just parse twice, once with the brackets and once without.
         * It's not possible that both will work.
         *
         * The problem is what happens when neither works: we need to
         * report an error, but which one will be better?
         *
         * It seems more likely that the user will be using a simple
         * target, so let's give the error for that case...
         *
         * First, try without the brackets, recording the error...
         */
        *target_value = g_variant_parse (NULL, target + 1, target + target_len - 1, NULL, error);

        if (*target_value == NULL)
          {
            /* If that failed, try with the brackets, ignoring the
             * error.
             */
            *target_value = g_variant_parse (NULL, target, target + target_len, NULL, NULL);

            /* If the second attempt failed, return the error from the
             * first attempt (which will already be set).
             */
            if (*target_value == NULL)
              goto bad_fmt;

            /* Otherwise, it worked, so clear the error. */
            g_clear_error (error);
          }
      }
      break;

    case '\0':
      *target_value = NULL;
      break;
    }

  *action_name = g_strndup (detailed_name, base_len);

  return TRUE;

bad_fmt:
  if (error)
    {
      if (*error == NULL)
        g_set_error (error, G_VARIANT_PARSE_ERROR, G_VARIANT_PARSE_ERROR_FAILED,
                     "Detailed action name '%s' has invalid format", detailed_name);
      else
        g_prefix_error (error, "Detailed action name '%s' has invalid format: ", detailed_name);
    }

  return FALSE;
}

static void
backport_g_markup_string_parser_start_element (GMarkupParseContext  *context,
                                               const gchar          *element_name,
                                               const gchar         **attribute_names,
                                               const gchar         **attribute_values,
                                               gpointer              user_data,
                                               GError              **error)
{
  g_set_error (error, G_MARKUP_ERROR, G_MARKUP_ERROR_UNKNOWN_ELEMENT,
               "Only text may appear inside element <%s> (not <%s>)",
               g_markup_parse_context_get_element (context), element_name);
}

typedef struct
{
  GString *str;
  gboolean translatable;
  gchar *context;
  gchar *domain;
} StringParserState;

static void
backport_g_markup_string_parser_text (GMarkupParseContext  *context,
                                      const gchar          *text,
                                      gsize                 text_len,
                                      gpointer              user_data,
                                      GError              **error)
{
  StringParserState *state = user_data;

  g_string_append_len (state->str, text, text_len);
}

static void
backport_g_markup_string_parser_error (GMarkupParseContext *context,
                                       GError              *error,
                                       gpointer             user_data)
{
  StringParserState *state = user_data;

  g_string_free (state->str, TRUE);
  g_free (state->context);
  g_free (state->domain);

  g_slice_free (StringParserState, state);
}

static void
backport_g_markup_string_parser_start (GMarkupParseContext  *context,
                                       gboolean              translatable,
                                       const gchar          *gettext_domain,
                                       const gchar          *gettext_context,
                                       GError              **error)
{
  static const GMarkupParser parser = {
    backport_g_markup_string_parser_start_element,
    NULL,
    backport_g_markup_string_parser_text,
    NULL,
    backport_g_markup_string_parser_error
  };
  StringParserState *state;

  if (translatable && gettext_domain == NULL)
    {
      g_set_error (error, G_MARKUP_ERROR, G_MARKUP_ERROR_MISSING_ATTRIBUTE,
                   "translation was requested for <%s> but no gettext domain was given",
                   g_markup_parse_context_get_element (context));
      return;
    }

  state = g_slice_new (StringParserState);
  state->str = g_string_new (NULL);
  state->domain = g_strdup (gettext_domain);
  state->context = g_strdup (gettext_context);
  state->translatable = translatable;

  g_markup_parse_context_push (context, &parser, state);
}

static gchar *
backport_g_markup_string_parser_end (GMarkupParseContext *context)
{
  StringParserState *state = g_markup_parse_context_pop (context);
  const gchar *translated;
  gchar *result;

  /* TODO: whitespace normalisation before translation? */

  if (state->translatable)
    {
      if (state->context)
        translated = g_dpgettext2 (state->domain, state->context, state->str->str);
      else
        translated = g_dgettext (state->domain, state->str->str);
    }
  else
    translated = state->str->str;

  if (translated != state->str->str)
    {
      g_string_free (state->str, TRUE);
      result = g_strdup (translated);
    }
  else
    result = g_string_free (state->str, FALSE);

  g_slice_free (StringParserState, state);

  return result;
}

static void
backport_g_markup_parser_reject_text (GMarkupParseContext  *context,
                                      const gchar          *text,
                                      gsize                 text_len,
                                      gpointer              user_data,
                                      GError              **error)
{
  gsize i;

  for (i = 0; i < text_len; i++)
    if (!g_ascii_isspace (text[i]))
      {
        g_set_error (error, G_MARKUP_ERROR, G_MARKUP_ERROR_INVALID_CONTENT,
                     "text may not appear inside <%s>",
                     g_markup_parse_context_get_element (context));
        break;
      }
}

typedef struct
{
  HudActionPublisher   *publisher;
  gchar                *gettext_domain;

  GVariantType         *type;

  gchar                *attribute;

  HudActionDescription *description;

  GVariantBuilder      *operation;

  GVariantBuilder      *widget;
} ParserState;

static void
start_element (GMarkupParseContext  *context,
               const gchar          *element_name,
               const gchar         **attribute_names,
               const gchar         **attribute_values,
               gpointer              user_data,
               GError              **error)
{
  ParserState *state = user_data;
  const GSList *element_stack;
  const gchar *container;

  element_stack = g_markup_parse_context_get_element_stack (context);
  container = element_stack->next ? element_stack->next->data : NULL;

#define COLLECT(first, ...) \
  g_markup_collect_attributes (element_name,                                 \
                               attribute_names, attribute_values, error,     \
                               first, __VA_ARGS__, G_MARKUP_COLLECT_INVALID)
#define OPTIONAL   G_MARKUP_COLLECT_OPTIONAL
#define STRDUP     G_MARKUP_COLLECT_STRDUP
#define STRING     G_MARKUP_COLLECT_STRING
#define BOOLEAN    G_MARKUP_COLLECT_BOOLEAN
#define NO_ATTRS() COLLECT (G_MARKUP_COLLECT_INVALID, NULL)

  if (container == NULL)
    {
      if (g_str_equal (element_name, "actions"))
        {
          COLLECT (OPTIONAL | STRDUP, "gettext-domain", &state->gettext_domain);
          return;
        }
    }

  else if (g_str_equal (container, "actions"))
    {
      if (g_str_equal (element_name, "action"))
        {
          const gchar *detailed_name;

          if (COLLECT (STRING, "name", &detailed_name))
            {
              gchar *action_name;
              GVariant *target;

              if (!backport_g_action_parse_detailed_name (detailed_name, &action_name, &target, error))
                return;

              state->description = hud_action_description_new (action_name, target);

              if (target)
                g_variant_unref (target);
              g_free (action_name);
            }

          return;
        }
    }

  else if (g_str_equal (container, "action"))
    {
      if (g_str_equal (element_name, "attribute"))
        {
          const gchar *typestr;
          const gchar *name;
          const gchar *gettext_context;
          gboolean translatable;

          if (COLLECT (STRDUP,             "name", &name,
                       OPTIONAL | BOOLEAN, "translatable", &translatable,
                       OPTIONAL | STRING,  "context", &gettext_context,
                       OPTIONAL | STRING,  "comments", NULL, /* ignore, just for translators */
                       OPTIONAL | STRING,  "type", &typestr))
            {
              if (typestr && !g_variant_type_string_is_valid (typestr))
                {
                  g_set_error (error, G_VARIANT_PARSE_ERROR,
                               G_VARIANT_PARSE_ERROR_INVALID_TYPE_STRING,
                               "Invalid GVariant type string '%s'", typestr);
                  return;
                }

              state->type = typestr ? g_variant_type_new (typestr) : NULL;
              state->attribute = g_strdup (name);

              backport_g_markup_string_parser_start (context, translatable, state->gettext_domain, gettext_context, error);
            }

          return;
        }

      else if (g_str_equal (element_name, "operation"))
        {
          if (NO_ATTRS ())
            state->operation = g_variant_builder_new (G_VARIANT_TYPE ("aa{sv}"));

          return;
        }
    }

  else if (g_str_equal (container, "operation"))
    {
      if (g_str_equal (element_name, "widget"))
        {
          const gchar *action;
          const gchar *type;

          if (COLLECT (STRING, "type",   &type,
                       STRING, "action", &action))
            {
              state->widget = g_variant_builder_new (G_VARIANT_TYPE_VARDICT);
              g_variant_builder_add (state->widget, "{sv}", "type", g_variant_new_string (type));
              g_variant_builder_add (state->widget, "{sv}", "action", g_variant_new_string (action));
            }

          return;
        }
    }

  else if (g_str_equal (container, "widget"))
    {
      if (g_str_equal (element_name, "label"))
        {
          const gchar *gettext_context;
          gboolean translatable;

          if (COLLECT (OPTIONAL | BOOLEAN, "translatable", &translatable,
                       OPTIONAL | STRING,  "context", &gettext_context,
                       OPTIONAL | STRING,  "comments", NULL))
            backport_g_markup_string_parser_start (context, translatable, state->gettext_domain, gettext_context, error);

          return;
        }

      else if (g_str_equal (element_name, "range"))
        {
          const gchar *minstr, *maxstr, *typestr;

          if (COLLECT (STRING,            "min",  &minstr,
                       STRING,            "max",  &maxstr,
                       OPTIONAL | STRING, "type", &typestr))
            {
              const GVariantType *type;
              GVariant *min, *max;

              if (typestr && !g_variant_type_string_is_valid (typestr))
                {
                  g_set_error (error, G_VARIANT_PARSE_ERROR,
                               G_VARIANT_PARSE_ERROR_INVALID_TYPE_STRING,
                               "Invalid GVariant type string '%s'", typestr);
                  return;
                }

              type = typestr ? G_VARIANT_TYPE (typestr) : NULL;

              min = g_variant_parse (type, minstr, NULL, NULL, error);
              if (min == NULL)
                {
                  g_prefix_error (error, "Parsing minimum value %s: ", minstr);
                  return;
                }

              /* Force both values to have the same type.
               *
               * If the type was explicitly specified then the first
               * value has the same type anyway, so this is a no-op.
               */
              type = g_variant_get_type (min);
              max = g_variant_parse (type, maxstr, NULL, NULL, error);
              if (max == NULL)
                {
                  g_prefix_error (error, "Parsing maximum value %s: ", maxstr);
                  g_variant_unref (min);
                  return;
                }

              g_variant_builder_add (state->widget, "{sv}", "range", g_variant_new ("(**)", min, max));
              g_variant_unref (max);
              g_variant_unref (min);
              return;
            }
        }
    }

  if (container)
    g_set_error (error, G_MARKUP_ERROR, G_MARKUP_ERROR_UNKNOWN_ELEMENT,
                 "Element <%s> not allowed inside <%s>",
                 element_name, container);
  else
    g_set_error (error, G_MARKUP_ERROR, G_MARKUP_ERROR_UNKNOWN_ELEMENT,
                 "Element <%s> not allowed at toplevel", element_name);
}

static void
end_element (GMarkupParseContext  *context,
             const gchar          *element_name,
             gpointer              user_data,
             GError              **error)
{
  ParserState *state = user_data;

  if (g_str_equal (element_name, "actions"))
    {
      g_free (state->gettext_domain);
      state->gettext_domain = NULL;
    }

  else if (g_str_equal (element_name, "action"))
    {
      hud_action_publisher_add_description (state->publisher, state->description);
      g_object_unref (state->description);
      state->description = NULL;
    }

  else if (g_str_equal (element_name, "attribute"))
    {
      GVariant *value;
      gchar *str;

      str = backport_g_markup_string_parser_end (context);

      if (state->type == NULL)
        /* No type string specified -> it's a normal string. */
        hud_action_description_set_attribute (state->description, state->attribute, "s", str);

      /* Else, we try to parse it according to the type string.  If
       * error is set here, it will follow us out, ending the parse.
       *
       * We still need to free everything, though, so ignore it here.
       */
      else if ((value = g_variant_parse (state->type, str, NULL, NULL, error)))
        {
          hud_action_description_set_attribute_value (state->description, state->attribute, value);
          g_variant_unref (value);
        }

      if (state->type)
        {
          g_variant_type_free (state->type);
          state->type = NULL;
        }

      g_free (state->attribute);
      state->attribute = NULL;
      g_free (str);
    }

  else if (g_str_equal (element_name, "operation"))
    {
      hud_action_description_set_attribute (state->description, "operation", "aa{sv}", state->operation);
      g_variant_builder_unref (state->operation);
      state->operation = NULL;
    }

  else if (g_str_equal (element_name, "widget"))
    {
      g_variant_builder_add (state->operation, "a{sv}", state->widget);
      g_variant_builder_unref (state->widget);
      state->widget = NULL;
    }

  else if (g_str_equal (element_name, "label"))
    {
      gchar *str;

      str = backport_g_markup_string_parser_end (context);
      g_variant_builder_add (state->widget, "{sv}", "label", g_variant_new_string (str));
      g_free (str);
    }
}

void
hud_action_publisher_add_actions_from_file (HudActionPublisher *publisher,
                                            const gchar        *filename)
{
  GMarkupParser parser = {
    start_element,
    end_element,
    backport_g_markup_parser_reject_text
  };
  GMarkupParseContext *context;
  GError *error = NULL;
  ParserState *state;
  gchar *contents;
  gsize size;

  state = g_slice_new0 (ParserState);
  state->publisher = publisher;

  if (!g_file_get_contents (filename, &contents, &size, &error))
    g_error ("Failed to open file %s: %s", filename, error->message);

  context = g_markup_parse_context_new (&parser, 0, state, NULL);
  if (!g_markup_parse_context_parse (context, contents, size, &error) ||
      !g_markup_parse_context_end_parse (context, &error))
    g_error ("Failed to parse action description XML from %s: %s", filename, error->message);
  g_markup_parse_context_free (context);
  g_free (contents);

  /* We know that everything has been cleaned up properly because
   * otherwise we would have hit the g_error() above.
   */
}

typedef GObjectClass HudActionDescriptionClass;

struct _HudActionDescription
{
  GObject parent_instance;

  gchar *action;
  GVariant *target;
  GHashTable *attrs;
};

guint hud_action_description_changed_signal;

G_DEFINE_TYPE (HudActionDescription, hud_action_description, G_TYPE_OBJECT)

static void
hud_action_description_finalize (GObject *object)
{
  HudActionDescription *description = HUD_ACTION_DESCRIPTION (object);

  g_free (description->action);
  if (description->target)
    g_variant_unref (description->target);
  g_hash_table_unref (description->attrs);

  G_OBJECT_CLASS (hud_action_description_parent_class)
    ->finalize (object);
}

static void
hud_action_description_init (HudActionDescription *description)
{
  description->attrs = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, (GDestroyNotify) g_variant_unref);
}

static void
hud_action_description_class_init (HudActionDescriptionClass *class)
{
  class->finalize = hud_action_description_finalize;

  hud_action_description_changed_signal = g_signal_new ("changed", HUD_TYPE_ACTION_DESCRIPTION,
                                                        G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED, 0, NULL, NULL,
                                                        g_cclosure_marshal_VOID__STRING,
                                                        G_TYPE_NONE, 1, G_TYPE_STRING);
}

HudActionDescription *
hud_action_description_new (const gchar *action_name,
                            GVariant    *action_target)
{
  HudActionDescription *description;

  description = g_object_new (HUD_TYPE_ACTION_DESCRIPTION, NULL);
  description->action = g_strdup (action_name);
  description->target = action_target ? g_variant_ref_sink (action_target) : NULL;

  return description;
}

void
hud_action_description_set_attribute_value (HudActionDescription *description,
                                            const gchar          *attribute_name,
                                            GVariant             *value)
{
  /* Don't allow setting the action or target as these form the
   * identity of the description and are stored separately...
   */
  g_return_if_fail (!g_str_equal (attribute_name, "action"));
  g_return_if_fail (!g_str_equal (attribute_name, "target"));

  if (value)
    g_hash_table_insert (description->attrs, g_strdup (attribute_name), g_variant_ref_sink (value));
  else
    g_hash_table_remove (description->attrs, attribute_name);

  g_signal_emit (description, hud_action_description_changed_signal,
                 g_quark_try_string (attribute_name), attribute_name);
}

void
hud_action_description_set_attribute (HudActionDescription *description,
                                      const gchar          *attribute_name,
                                      const gchar          *format_string,
                                      ...)
{
  GVariant *value;

  if (format_string != NULL)
    {
      va_list ap;

      va_start (ap, format_string);
      value = g_variant_new_va (format_string, NULL, &ap);
      va_end (ap);
    }
  else
    value = NULL;

  hud_action_description_set_attribute_value (description, attribute_name, value);
}

const gchar *
hud_action_description_get_action_name (HudActionDescription *description)
{
  return description->action;
}

GVariant *
hud_action_description_get_action_target (HudActionDescription *description)
{
  return description->target;
}
