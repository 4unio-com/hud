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
 */

#define G_LOG_DOMAIN "hudrandomsource"

#include <glib.h>

#include "hudquery.h"
#include "hudtoken.h"
#include "hudsource.h"
#include "hudrandomsource.h"
#include "word-list.h"

#define MAX_DEPTH 6
#define MAX_ITEMS 20
#define MAX_WORDS 4

/* Longest word in the word-list (upper bound) */
#define MAX_LETTERS 20

struct _HudRandomSource
{
  GObject parent_instance;

  /* instance members */
  GHashTable *items;

  /* Max nested depth of menu items */
  gint max_depth;

  /* Max number of items per submenu */
  gint max_items;

  /* Max number of words per label.
   * NB: keep MAX_WORDS * MAX_DEPTH under 32
   */
  gint max_words;
};

typedef GObjectClass HudRandomSourceClass;

static void hud_random_source_iface_init (HudSourceInterface *iface);
G_DEFINE_TYPE_WITH_CODE (HudRandomSource, hud_random_source, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (HUD_TYPE_SOURCE, hud_random_source_iface_init))

void
hud_random_source_search (HudSource    *hud_source,
    HudTokenList *search_tokens,
    SearchFlags   flags,
    void        (*append_func) (HudResult * result, gpointer user_data),
    gpointer      user_data)
{
  HudRandomSource *source = (HudRandomSource *) hud_source;
  GHashTableIter iter;
  gpointer item;

  g_hash_table_iter_init (&iter, source->items);
  while (g_hash_table_iter_next (&iter, &item, NULL))
    {
      HudResult *result;

      result = hud_result_get_if_matched (item, search_tokens, 0);
      if (result)
        append_func(result, user_data);
    }
}

static HudSource *
hud_random_source_get (HudSource    *hud_source,
                       const gchar *application_id)
{
  return NULL;
}

static void
hud_random_source_ignore_use (HudSource *source)
{
}

static gchar *
hud_random_source_make_word (GRand *rand,
           gchar *buffer)
{
  const gchar *word;
  gint choice;

  choice = g_rand_int_range (rand, 0, G_N_ELEMENTS (word_list));
  word = word_list[choice];

  while (*word)
    *buffer++ = *word++;

  return buffer;
}

gchar *
hud_random_source_make_words (GRand *rand,
            gint   n_words)
{
  gchar *buffer;
  gchar *ptr;
  gint i;

  buffer = g_malloc ((MAX_LETTERS + 1) * n_words);

  ptr = buffer;
  for (i = 0; i < n_words; i++)
    {
      if (i)
        *ptr++ = ' ';

      ptr = hud_random_source_make_word (rand, ptr);
    }

  *ptr = '\0';

  return buffer;
}

static HudStringList *
hud_random_source_make_name (HudRandomSource *self,
                         GRand         *rand,
                         HudStringList *context)
{
  HudStringList *name;
  gchar *label;

  label = hud_random_source_make_words (rand, g_rand_int_range (rand, 1, self->max_words + 1));
  name = hud_string_list_cons (label, context);
  g_free (label);

  return name;
}

static void
hud_random_source_populate_table (HudRandomSource *self,
                              GRand         *rand,
                              HudStringList *context,
                              gint           depth)
{
  gint n_items;
  gint i;

  n_items = g_rand_int_range (rand, 1, self->max_items + 1);

  for (i = 0; i < n_items; i++)
    {
      HudStringList *name;
      gboolean is_submenu;
      HudItem *item;

      name = hud_random_source_make_name (self, rand, context);

      if (depth != self->max_depth)
        /* Decrease the chances of a particular item being a submenu as we
         * go deeper into the menu structure.
         */
        is_submenu = g_rand_int_range (rand, 0, depth + 1) == 0;
      else
        /* At the maximum depth, prevent any items from being submenus. */
        is_submenu = FALSE;

      item = hud_item_new (name, name, "", NULL, NULL, !is_submenu);
      g_hash_table_add (self->items, item);

      if (is_submenu)
        hud_random_source_populate_table (self, rand, name, depth + 1);

      hud_string_list_unref (name);
    }
}

static void
hud_random_source_finalize (GObject *object)
{
  HudRandomSource *source = (HudRandomSource *) object;

  g_hash_table_unref (source->items);

  G_OBJECT_CLASS (hud_random_source_parent_class)
    ->finalize (object);
}

static void
hud_random_source_init (HudRandomSource *source)
{
  source->items = g_hash_table_new_full (NULL, NULL, g_object_unref, NULL);
}

static void
hud_random_source_iface_init (HudSourceInterface *iface)
{
  iface->use = hud_random_source_ignore_use;
  iface->unuse = hud_random_source_ignore_use;
  iface->search = hud_random_source_search;
  iface->get = hud_random_source_get;
}

static void
hud_random_source_class_init (HudRandomSourceClass *class)
{
  class->finalize = hud_random_source_finalize;
}

HudSource *
hud_random_source_new (GRand *rand)
{
  return hud_random_source_new_full (rand, MAX_DEPTH, MAX_ITEMS, MAX_WORDS);
}

HudSource *
hud_random_source_new_full (GRand *rand, const gint max_depth, const gint max_items,
    const gint max_words)
{
  HudRandomSource *source;

  source = g_object_new (HUD_TYPE_RANDOM_SOURCE, NULL);

  source->max_depth = max_depth;
  source->max_items = max_items;
  source->max_words = max_words;

  hud_random_source_populate_table (source, rand, NULL, 0);

  return HUD_SOURCE (source);
}
