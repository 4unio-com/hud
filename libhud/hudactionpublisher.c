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
#include <string.h>

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
  GSequence *descriptions;
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

typedef GObjectClass HudActionDescriptionClass;

struct _HudActionDescription
{
  GObject parent_instance;

  gchar *identifier;
  gchar *action;
  GVariant *target;
  GHashTable *attrs;
};

guint hud_action_description_changed_signal;

G_DEFINE_TYPE (HudActionDescription, hud_action_description, G_TYPE_OBJECT)


static void
hud_action_publisher_finalize (GObject *object)
{
  g_error ("g_object_unref() called on internally-owned ref of HudActionPublisher");
}

static void
hud_action_publisher_init (HudActionPublisher *publisher)
{
  publisher->descriptions = g_sequence_new (g_object_unref);
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

/**
 * hud_action_publisher_get:
 *
 * Gets the process-wide singleton instance of the #HudActionPublisher.
 *
 * Returns: (transfer none): the #HudActionPublisher
 **/
HudActionPublisher *
hud_action_publisher_get (void)
{
  static HudActionPublisher *publisher;

  if (!publisher)
    publisher = g_object_new (HUD_TYPE_ACTION_PUBLISHER, NULL);

  return publisher;
}

static gchar *
format_identifier (const gchar *action_name,
                   GVariant    *action_target)
{
  gchar *targetstr;
  gchar *identifier;

  if (action_target)
    {
      targetstr = g_variant_print (action_target, TRUE);
      identifier = g_strdup_printf ("%s(%s)", action_name, targetstr);
      g_free (targetstr);
    }

  else
    identifier = g_strdup_printf ("%s()", action_name);

  return identifier;
}

static gint
compare_descriptions (gconstpointer a,
                      gconstpointer b,
                      gpointer      user_data)
{
  const HudActionDescription *da = a;
  const HudActionDescription *db = b;

  return strcmp (da->identifier, db->identifier);
}

static void
description_changed (HudActionDescription *description,
                     const gchar          *attribute_name,
                     gpointer              user_data)
{
  HudActionPublisher *publisher = user_data;
  GSequenceIter *iter;

  iter = g_sequence_lookup (publisher->descriptions, description, compare_descriptions, NULL);
  g_assert (g_sequence_get (iter) == description);

  g_menu_model_items_changed (G_MENU_MODEL (publisher->aux), g_sequence_iter_get_position (iter), 1, 1);
}

static void
disconnect_handler (gpointer data,
                    gpointer user_data)
{
  HudActionPublisher *publisher = user_data;
  HudActionDescription *description = data;

  g_signal_handlers_disconnect_by_func (description, description_changed, publisher);
}

#define g_menu_model_items_changed(x,...)

/**
 * hud_action_publisher_add_description:
 * @publisher: the #HudActionPublisher
 * @description: an action description
 *
 * Adds @description to the list of actions that the application exports
 * to the HUD.
 *
 * If the application is already exporting an action with the same name
 * and target value as @description then it will be replaced.
 *
 * You should only use this API for situations like recent documents and
 * bookmarks.  Most actions in an application are more static in nature
 * and are better handled with
 * hud_action_publisher_add_descriptions_from_file().
 */
void
hud_action_publisher_add_description (HudActionPublisher   *publisher,
                                      HudActionDescription *description)
{
  GSequenceIter *iter;

  iter = g_sequence_lookup (publisher->descriptions, description, compare_descriptions, NULL);

  if (g_hash_table_lookup (description->attrs, "operation"))
    g_print ("operation: %s\n", g_variant_print (g_hash_table_lookup (description->attrs, "operation"), TRUE));

  if (iter == NULL)
    {
      /* We are not replacing -- add new. */
      iter = g_sequence_insert_sorted (publisher->descriptions, description, compare_descriptions, NULL);

      /* Signal that we added the items */
      g_menu_model_items_changed (G_MENU_MODEL (publisher->aux), g_sequence_iter_get_position (iter), 0, 1);
    }
  else
    {
      /* We are replacing an existing item. */
      disconnect_handler (g_sequence_get (iter), publisher);
      g_sequence_set (iter, description);

      /* A replace is 1 remove and 1 add */
      g_menu_model_items_changed (G_MENU_MODEL (publisher->aux), g_sequence_iter_get_position (iter), 1, 1);
    }

  g_signal_connect (description, "changed", G_CALLBACK (description_changed), publisher);
}

/**
 * hud_action_publisher_remove_description:
 * @publisher: the #HudActionPublisher
 * @action_name: an action name
 * @action_target: (allow none): an action target
 *
 * Removes the action descriptions that has the name @action_name and
 * the target value @action_target (including the possibility of %NULL).
 **/
void
hud_action_publisher_remove_description (HudActionPublisher *publisher,
                                         const gchar        *action_name,
                                         GVariant           *action_target)
{
  HudActionDescription tmp;
  GSequenceIter *iter;

  tmp.identifier = format_identifier (action_name, action_target);
  iter = g_sequence_lookup (publisher->descriptions, &tmp, compare_descriptions, NULL);
  g_free (tmp.identifier);

  if (iter)
    {
      gint position;

      position = g_sequence_iter_get_position (iter);
      disconnect_handler (g_sequence_get (iter), publisher);
      g_sequence_remove (iter);

      g_menu_model_items_changed (G_MENU_MODEL (publisher->aux), position, 1, 0);
    }
}

/**
 * hud_action_publisher_remove_descriptions:
 * @publisher: the #HudActionPublisher
 * @action_name: an action name
 *
 * Removes all action descriptions that has the name @action_name and
 * any target value.
 **/
void
hud_action_publisher_remove_descriptions (HudActionPublisher *publisher,
                                          const gchar        *action_name)
{
  HudActionDescription before, after;
  GSequenceIter *start, *end;

  before.identifier = (gchar *) action_name;
  after.identifier = g_strconcat (action_name, "~", NULL);
  start = g_sequence_search (publisher->descriptions, &before, compare_descriptions, NULL);
  end = g_sequence_search (publisher->descriptions, &after, compare_descriptions, NULL);
  g_free (after.identifier);

  if (start != end)
    {
      gint s, e;

      s = g_sequence_iter_get_position (start);
      e = g_sequence_iter_get_position (end);
      g_sequence_foreach_range (start, end, disconnect_handler, publisher);
      g_sequence_remove_range (start, end);

      g_menu_model_items_changed (G_MENU_MODEL (publisher->aux), s, e - s, 0);
    }
}

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

/**
 * hud_action_publisher_add_descriptions_from_file:
 * @publisher: the #HudActionPublisher
 * @filename: the filename of the XML file describing the actions
 *
 * Adds action descriptions from an XML file.
 *
 * If any error occurs while attempting to open or parse the file, the
 * program will abort.
 **/
void
hud_action_publisher_add_descriptions_from_file (HudActionPublisher *publisher,
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

/**
 * HudActionDescription:
 *
 * A description of an action that is accessible from the HUD.  This is
 * an opaque structure type and all accesses must be made via the API.
 **/

static void
hud_action_description_finalize (GObject *object)
{
  HudActionDescription *description = HUD_ACTION_DESCRIPTION (object);

  g_free (description->identifier);
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

/**
 * hud_action_description_new:
 * @action_name: a (namespaced) action name
 * @action_target: an action target
 *
 * Creates a new #HudActionDescription.
 *
 * The situations in which you want to do this are limited to "dynamic"
 * types of actions -- things like bookmarks or recent documents.
 *
 * Use hud_action_publisher_add_descriptions_from_file() to take care of
 * the bulk of the actions in your application.
 *
 * Returns: a new #HudActionDescription with no attributes
 **/
HudActionDescription *
hud_action_description_new (const gchar *action_name,
                            GVariant    *action_target)
{
  HudActionDescription *description;

  description = g_object_new (HUD_TYPE_ACTION_DESCRIPTION, NULL);
  description->action = g_strdup (action_name);
  description->target = action_target ? g_variant_ref_sink (action_target) : NULL;
  description->identifier = format_identifier (action_name, action_target);

  return description;
}

/**
 * hud_action_description_set_attribute_value:
 * @description: a #HudActionDescription
 * @attribute_name: an attribute name
 * @value: (allow none): the new value for the attribute
 *
 * Sets or unsets an attribute on @description.
 *
 * You may not change the "action" or "target" attributes.
 *
 * If @value is non-%NULL then it is the new value for attribute.  A
 * %NULL @value unsets @attribute_name.
 *
 * @value is consumed if it is floating.
 **/
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

/**
 * hud_action_description_set_attribute:
 * @description: a #HudActionDescription
 * @attribute_name: an attribute name
 * @format_string: (allow none): a #GVariant format string
 * @...: arguments to @format_string
 *
 * Sets or unsets an attribute on @description.
 *
 * You may not change the "action" or "target" attributes.
 *
 * If @format_string is non-%NULL then this call is equivalent to
 * g_variant_new() and hud_action_description_set_attribute_value().
 *
 * If @format_string is %NULL then this call is equivalent to
 * hud_action_description_set_attribute_value() with a %NULL value.
 **/
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

/**
 * hud_action_description_get_action_name:
 * @description: a #HudActionDescription
 *
 * Gets the action name of @description.
 *
 * This, together with the action target, uniquely identify an action
 * description.
 *
 * Returns: (transfer none): the action name
 **/
const gchar *
hud_action_description_get_action_name (HudActionDescription *description)
{
  return description->action;
}

/**
 * hud_action_description_get_action_target:
 * @description: a #HudActionDescription
 *
 * Gets the action target of @description (ie: the #GVariant that will
 * be passed to invocations of the action).
 *
 * This may be %NULL.
 *
 * This, together with the action name, uniquely identify an action
 * description.
 *
 * Returns: (transfer none): the target value
 **/
GVariant *
hud_action_description_get_action_target (HudActionDescription *description)
{
  return description->target;
}
