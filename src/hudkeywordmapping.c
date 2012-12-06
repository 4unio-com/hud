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

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#include "hudkeywordmapping.h"
#include "config.h"

struct _HudKeywordMapping
{
  GObject parent_instance;

  /* instance members */

  GHashTable *mappings;
};

typedef GObjectClass HudKeywordMappingClass;

G_DEFINE_TYPE(HudKeywordMapping, hud_keyword_mapping, G_TYPE_OBJECT);

static gint
hud_keyword_mapping_load_xml(HudKeywordMapping* self, const char* filename,
    const xmlChar* xpathExpr);

static void
hud_keyword_mapping_load_xml_mappings(HudKeywordMapping* self, xmlNodeSetPtr nodes);

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
hud_keyword_mapping_init (HudKeywordMapping *self)
{
  self->mappings = g_hash_table_new(g_str_hash, g_str_equal);
}

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

void
hud_keyword_mapping_load (HudKeywordMapping *self,
    const gchar *desktop_file, const gchar *datadir, const gchar *localedir)
{
  /* Now expand the label into multiple keywords */
  gchar *basename = g_path_get_basename(desktop_file);
  gchar **names = g_strsplit(basename, ".", 0);
  gchar *mapping_filename = g_strdup_printf("%s.xml", names[0]);
  gchar *mapping_path = g_build_filename(datadir, "hud", "keywords",
      mapping_filename, NULL );
  const xmlChar* expr = (xmlChar*) "//keywordMapping/mapping";

  /* Set the textdomain to match the program we're translating */
  g_debug("Setting textdomain = (\"%s\", \"%s\")", names[0], localedir);
  bindtextdomain (names[0], localedir);
  textdomain (names[0]);

  /* Do the main job */
  if (hud_keyword_mapping_load_xml(self, mapping_path, expr) < 0)
  {
    g_warning("Unable to load %s", mapping_path);
  }

  g_free(basename);
  g_strfreev(names);
  g_free(mapping_filename);
  g_free(mapping_path);

  /* Restore the original textdomain */
  textdomain(GETTEXT_PACKAGE);
}

/*
 * Do not free the GPtrArray* returned by this method.
 */
GPtrArray* hud_keyword_mapping_transform(HudKeywordMapping *self, const gchar* label)
{
  GPtrArray* results;

  results = g_hash_table_lookup (self->mappings, label);

  if (!results)
  {
    results = g_ptr_array_new();
    g_ptr_array_add(results, g_strdup(label));
    g_hash_table_insert(self->mappings, (gpointer) label, (gpointer) results);

  }

  return results;
}

/**
 * hud_keyword_mapping_load_xml:
 * @self: The #HudKeywordMapping
 * @filename:   the input XML filename.
 * @xpathExpr:    the xpath expression for evaluation.
 *
 * Parses input XML file, evaluates XPath expression and loads the keyword hash.
 *
 * Returns 0 on success and a negative value otherwise.
 */
static gint
hud_keyword_mapping_load_xml(HudKeywordMapping* self,
    const gchar* filename, const xmlChar* xpathExpr)
{
  xmlDocPtr doc;
  xmlXPathContextPtr xpathCtx;
  xmlXPathObjectPtr xpathObj;

  g_assert(filename);
  g_assert(xpathExpr);

  /* Load XML document */
  doc = xmlParseFile(filename);
  if (doc == NULL )
  {
    g_warning("Unable to parse file \"%s\"", filename);
    return (-1);
  }

  /* Create xpath evaluation context */
  xpathCtx = xmlXPathNewContext(doc);
  if (xpathCtx == NULL )
  {
    g_warning("Unable to create new XPath context");
    xmlFreeDoc(doc);
    return (-1);
  }

  /* Evaluate xpath expression */
  xpathObj = xmlXPathEvalExpression(xpathExpr, xpathCtx);
  if (xpathObj == NULL )
  {
    g_warning("Unable to evaluate xpath expression \"%s\"", xpathExpr);
    xmlXPathFreeContext(xpathCtx);
    xmlFreeDoc(doc);
    return (-1);
  }

  /* Load results */
  hud_keyword_mapping_load_xml_mappings(self, xpathObj->nodesetval);

  /* Cleanup */
  xmlXPathFreeObject(xpathObj);
  xmlXPathFreeContext(xpathCtx);
  xmlFreeDoc(doc);

  return (0);
}

/**
 * hud_keyword_mapping_load_xml_mappings:
 * @self:     the keyword mapping to populate
 * @nodes:    set of result nodes giving us keywords
 *
 * Loads the keywords from the given XML result set.
 */
static void
hud_keyword_mapping_load_xml_mappings(HudKeywordMapping* self,
    xmlNodeSetPtr nodes)
{
  xmlNodePtr cur, child;
  gint size, i, j;
  size = (nodes) ? nodes->nodeNr : 0;

  for (i = 0; i < size; ++i)
  {
    g_assert(nodes->nodeTab[i]);

    if (nodes->nodeTab[i]->type == XML_ELEMENT_NODE)
    {
      gchar *original, *original_to_lookup, *original_translated, *keywords_translated;
      GPtrArray *keywords = g_ptr_array_new_full(8, g_free);
      cur = nodes->nodeTab[i];

      original = (gchar*) xmlGetProp (cur, (xmlChar*) "original");
      original_translated = _((gchar*) original);

      /* First try to translate hud-keywords:original. If it comes back without
       * translation, then read the keywords from the XML file.
       */
      original_to_lookup = g_strconcat ("hud-keywords:", original, NULL );
      keywords_translated = gettext(original_to_lookup);
      if (original_to_lookup == keywords_translated)
      {
        /* Go through each of the keywords */
        for (child = cur->children; child; child = child->next)
        {
          if (child->type == XML_ELEMENT_NODE)
          {
            xmlChar *keyword = xmlGetProp (child, (xmlChar*) "name");
            g_ptr_array_add (keywords, (gpointer) keyword);
          }
        }
      }
      else
      {
        gchar** split = g_strsplit (keywords_translated, ";", 0);

        /* Go through each of the keywords */
        for (j = 0; split[j] != NULL; j++)
        {
          gchar* keyword = g_strstrip(split[j]);
          g_ptr_array_add (keywords, (gpointer) g_strdup(keyword));
        }

        g_strfreev (split);
      }

      g_hash_table_insert (self->mappings, (gpointer) original_translated,
          (gpointer) keywords);

      if (original != original_translated)
        g_free (original);
      g_free (original_to_lookup);
    }
  }
}
