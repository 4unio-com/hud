/*
 * Copyright © 2012 Canonical Ltd.
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
 * Authors: Pete Woods <pete.woods@canonical.com>
 */

#define G_LOG_DOMAIN "hudkeywordmapping"

#include <glib/gi18n.h>

#include "hudkeywordmapping.h"
#include "config.h"

/**
 * SECTION:hudkeywordmapping
 * @title: HudKeywordMapping
 * @short_description: Provides additional keywords for user actions.
 *
 * The #HudKeywordMapping service parses an XML file that provides a
 * mapping for additional keywords that a user action can match on.
 *
 * It also consults gettext to see if a translation of the form
 * "hud-keywords:<original action>" -> "keyword 1; keyword 2; keyword 3"
 * exists.
 *
 * An instance of #HudKeywordMapping is created for each separate
 * application.
 **/

struct _HudKeywordMapping
{
  GObject parent_instance;

  /* Hash table storing the keyword mappings */
  GHashTable *mappings;
};

typedef GObjectClass HudKeywordMappingClass;

G_DEFINE_TYPE(HudKeywordMapping, hud_keyword_mapping, G_TYPE_OBJECT);

static gint
hud_keyword_mapping_load_xml(HudKeywordMapping* self, const char* filename);

static void
hud_keyword_mapping_start_element (GMarkupParseContext *context,
    const gchar *element_name, const gchar **attribute_names,
    const gchar **attribute_values, gpointer user_data, GError **error);

static void
hud_keyword_mapping_end_element (GMarkupParseContext *context,
    const gchar *element_name, gpointer user_data, GError **error);

static void
hud_keyword_mapping_finalize (GObject *object)
{
  HudKeywordMapping *self = HUD_KEYWORD_MAPPING(object);

  g_hash_table_destroy(self->mappings);

  G_OBJECT_CLASS (hud_keyword_mapping_parent_class)
    ->finalize (object);
}

static void
hud_keyword_mapping_class_init (HudKeywordMappingClass *klass)
{
  klass->finalize = hud_keyword_mapping_finalize;
}

static void
hud_keyword_mapping_key_destroyed (gpointer data)
{
  g_free ((gchar*) data);
}

static void
hud_keyword_mapping_value_destroyed (gpointer data)
{
  g_ptr_array_free((GPtrArray*) data, TRUE);
}

static void
hud_keyword_mapping_init (HudKeywordMapping *self)
{
  self->mappings = g_hash_table_new_full (g_str_hash, g_str_equal,
      (GDestroyNotify) hud_keyword_mapping_key_destroyed,
      (GDestroyNotify) hud_keyword_mapping_value_destroyed);
}

typedef enum
{
  HUD_KEYWORD_START,
  HUD_KEYWORD_IN_KEYWORD_MAPPING,
  HUD_KEYWORD_IN_MAPPING,
  HUD_KEYWORD_IN_KEYWORD,
  HUD_KEYWORD_IN_UNKNOWN
} HudKeywordMappingParserState;

typedef struct
{
  GHashTable *mappings;
  HudKeywordMappingParserState state;
  gchar *original_translated;
  GPtrArray *keywords;
  gint translation_found;
} HudKeywordMappingParser;

static GMarkupParser hud_keyword_mapping_parser =
{ hud_keyword_mapping_start_element, hud_keyword_mapping_end_element,
    NULL, NULL, NULL };

/**
 * hud_keyword_mapping_new:
 *
 * Creates a #HudKeywordMapping.
 *
 * Returns: a new empty #HudKeywordMapping
 **/
HudKeywordMapping *
hud_keyword_mapping_new (void)
{
  return g_object_new (HUD_TYPE_KEYWORD_MAPPING, NULL);
}

/**
 * hud_keyword_mapping_load:
 *
 * Loads keyword mappings from the specified XML file for the given
 * application. The XML file is searched for in the path
 * "<datadir>/hud/keywords".
 *
 * Translations are performed using the locale information
 * provided.
 *
 * @self: Instance of #HudKeywordMapping
 * @app_id: Used to determine the identity of the application the mappings are for
 * @datadir: The base directory that HUD data is stored in.
 * @localedir: The directory to load the gettext locale from.
 */
void
hud_keyword_mapping_load (HudKeywordMapping *self,
    const gchar *app_id, const gchar *datadir, const gchar *localedir)
{
  /* Now expand the label into multiple keywords */
  gchar *mapping_filename = g_strdup_printf("%s.xml", app_id);
  gchar *mapping_path = g_build_filename(datadir, "hud", "keywords",
      mapping_filename, NULL );

  /* Set the textdomain to match the program we're translating */
  g_debug("Setting textdomain = (\"%s\", \"%s\")", app_id, localedir);
  bindtextdomain (app_id, localedir);
  textdomain (app_id);

  /* Do the main job */
  if (hud_keyword_mapping_load_xml(self, mapping_path) < 0)
  {
    g_debug("Unable to load %s", mapping_path);
  }

  g_free(mapping_filename);
  g_free(mapping_path);

  /* Restore the original textdomain */
  textdomain(GETTEXT_PACKAGE);
}

/*
 * hud_keyword_mapping_transform:
 *
 * Consults the table of keywords and provides the list of keywords for given action.
 *
 * @self: Instance of #HudKeywordMapping.
 * @label: The label of the action to provide keywords for.
 *
 * Returns: #GPtrArray of the keywords for the given action. Do not free
 * this result.
 */
GPtrArray*
hud_keyword_mapping_transform(HudKeywordMapping *self, const gchar* label)
{
  GPtrArray* results;

  results = g_hash_table_lookup (self->mappings, label);

  if (!results)
  {
    results = g_ptr_array_new_full(0, g_free);
    g_hash_table_insert(self->mappings, (gpointer) g_strdup(label), (gpointer) results);
  }

  return results;
}

/**
 * hud_keyword_mapping_load_xml:
 * @self: The #HudKeywordMapping
 * @filename:   the input XML filename.
 *
 * Parses input XML file, evaluates XPath expression and loads the keyword hash.
 *
 * Returns 0 on success and a negative value otherwise.
 */
static gint
hud_keyword_mapping_load_xml(HudKeywordMapping* self,
    const gchar* filename)
{
  gchar *text;
  gsize length;
  GMarkupParseContext *context;
  HudKeywordMappingParser parser = {self->mappings, HUD_KEYWORD_START, };

  g_assert(filename);

  if (g_file_get_contents (filename, &text, &length, NULL ) == FALSE)
  {
    g_debug("Unable to read XML file \"%s\"", filename);
    return (-1);
  }

  context = g_markup_parse_context_new (&hud_keyword_mapping_parser, 0, &parser, NULL );

  if (g_markup_parse_context_parse (context, text, length, NULL ) == FALSE)
  {
    g_warning("Unable to parse file \"%s\"", filename);
    g_free (text);
    g_markup_parse_context_free (context);
    return (-1);
  }

  g_free (text);
  g_markup_parse_context_free (context);

  return (0);
}

static gchar*
hud_keyword_mapping_get_attribute_value (const gchar* name,
    const gchar **attribute_names, const gchar **attribute_values)
{
  const gchar **name_cursor = attribute_names;
  const gchar **value_cursor = attribute_values;

  while (*name_cursor)
  {
    if (strcmp (*name_cursor, name) == 0)
      return g_strdup (*value_cursor);

    name_cursor++;
    value_cursor++;
  }

  return NULL;
}

static void
hud_keyword_mapping_start_keyword (HudKeywordMappingParser *parser,
    const gchar **attribute_names, const gchar **attribute_values)
{
  if (!parser->translation_found)
  {
    gchar* keyword = hud_keyword_mapping_get_attribute_value ("name", attribute_names,
                attribute_values);
    /* Check the keyword isn't empty */
    if(keyword && keyword[0] != '\0')
      g_ptr_array_add (parser->keywords, keyword);
  }
}

static void
hud_keyword_mapping_start_mapping (HudKeywordMappingParser *parser,
    const gchar **attribute_names, const gchar **attribute_values)
{
  gchar *original, *original_to_lookup, *keywords_translated;

  original = hud_keyword_mapping_get_attribute_value ("original",
      attribute_names, attribute_values);

  parser->original_translated = g_strdup (_((gchar*) original));
  parser->keywords = g_ptr_array_new_full (8, g_free);

  /* First try to translate hud-keywords:original. If it comes back without
   * translation, then read the keywords from the XML file.
   */
  original_to_lookup = g_strconcat ("hud-keywords:", original, NULL );
  keywords_translated = gettext (original_to_lookup);

  parser->translation_found = (original_to_lookup != keywords_translated);

  /* If we find a translation we can parse it here. Otherwise we'll have to wait
   * for the XML parser to move across each of our child elements.
   */
  if (parser->translation_found)
  {
    gint j;
    gchar** split = g_strsplit (keywords_translated, ";", 0);

    /* Go through each of the keywords */
    for (j = 0; split[j] != NULL ; j++)
    {
      gchar* keyword = g_strstrip(split[j]);
      g_ptr_array_add (parser->keywords, (gpointer) g_strdup (keyword));
    }

    g_strfreev (split);
  }

  g_free (original);
  g_free (original_to_lookup);
}

static void
hud_keyword_mapping_start_element (GMarkupParseContext *context, const gchar *element_name,
    const gchar **attribute_names, const gchar **attribute_values,
    gpointer user_data, GError **error)
{
  HudKeywordMappingParser *parser = (HudKeywordMappingParser*) user_data;

  switch (parser->state)
  {
  case HUD_KEYWORD_START:
    if (strcmp (element_name, "keywordMapping") == 0)
      parser->state = HUD_KEYWORD_IN_KEYWORD_MAPPING;
    else
      parser->state = HUD_KEYWORD_IN_UNKNOWN;
    break;

  case HUD_KEYWORD_IN_KEYWORD_MAPPING:
    if (strcmp (element_name, "mapping") == 0)
    {
      parser->state = HUD_KEYWORD_IN_MAPPING;
      hud_keyword_mapping_start_mapping (parser, attribute_names,
          attribute_values);
    }
    else
      parser->state = HUD_KEYWORD_IN_UNKNOWN;
    break;

  case HUD_KEYWORD_IN_MAPPING:
    if (strcmp (element_name, "keyword") == 0)
    {
      parser->state = HUD_KEYWORD_IN_KEYWORD;
      hud_keyword_mapping_start_keyword (parser, attribute_names,
                attribute_values);
    }
    else
      parser->state = HUD_KEYWORD_IN_UNKNOWN;
    break;

  case HUD_KEYWORD_IN_KEYWORD:
    g_warning("Shouldn't be entering any child elements from a keyword element.");
    parser->state = HUD_KEYWORD_IN_UNKNOWN;
    break;

  case HUD_KEYWORD_IN_UNKNOWN:
    g_error("Hud keyword parsing failed. In unknown state.");
    break;
  }
}

static void
hud_keyword_mapping_end_element (GMarkupParseContext *context, const gchar *element_name,
    gpointer user_data, GError **error)
{
  HudKeywordMappingParser *parser = (HudKeywordMappingParser*) user_data;

  switch (parser->state)
  {
  case HUD_KEYWORD_START:
    g_error("Should not be able to return to start state in keyword parser");
    break;

  case HUD_KEYWORD_IN_KEYWORD_MAPPING:
    parser->state = HUD_KEYWORD_START;
    break;

  case HUD_KEYWORD_IN_MAPPING:
    parser->state = HUD_KEYWORD_IN_KEYWORD_MAPPING;

    /* Build an entry from the collected keywords */
    g_hash_table_insert (parser->mappings,
        (gpointer) parser->original_translated, (gpointer) parser->keywords);
    break;

  case HUD_KEYWORD_IN_KEYWORD:
    parser->state = HUD_KEYWORD_IN_MAPPING;
    break;

  case HUD_KEYWORD_IN_UNKNOWN:
    g_warning("Hud keyword parsing failed. In unknown state.");
    break;
  }
}
