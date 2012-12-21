
#include <glib.h>

#include "hudquery.h"
#include "hudtoken.h"
#include "hudsource.h"
#include "hudrandomsource.h"
#include "word-list.h"

#define MAX_DEPTH 6
#define MAX_ITEMS 20
#define MAX_WORDS 4
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

  /* Longest word in the word-list (upper bound) */
  gint max_letters;
};

typedef GObjectClass HudRandomSourceClass;

static void hud_random_source_iface_init (HudSourceInterface *iface);
G_DEFINE_TYPE_WITH_CODE (HudRandomSource, hud_random_source, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (HUD_TYPE_SOURCE, hud_random_source_iface_init))

void
hud_random_source_search (HudSource    *hud_source,
    HudTokenList *search_tokens,
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

static gchar *
hud_random_source_make_words_full (GRand *rand,
            gint   n_words, const gint max_letters)
{
  gchar *buffer;
  gchar *ptr;
  gint i;

  buffer = g_malloc ((max_letters + 1) * n_words);

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

gchar *
hud_random_source_make_words (GRand *rand,
            gint   n_words)
{
  return hud_random_source_make_words_full(rand, n_words, MAX_LETTERS);
}

static HudStringList *
hud_random_source_make_name (HudRandomSource *self,
                         GRand         *rand,
                         HudStringList *context)
{
  HudStringList *name;
  gchar *label;

  label = hud_random_source_make_words_full (rand, g_rand_int_range (rand, 1, self->max_words + 1), self->max_letters);
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
}

static void
hud_random_source_class_init (HudRandomSourceClass *class)
{
  class->finalize = hud_random_source_finalize;
}

HudSource *
hud_random_source_new (GRand *rand)
{
  return hud_random_source_new_full (rand, MAX_DEPTH, MAX_ITEMS, MAX_WORDS,
      MAX_LETTERS);
}

HudSource *
hud_random_source_new_full (GRand *rand, const gint max_depth, const gint max_items,
    const gint max_words, const gint max_letters)
{
  HudRandomSource *source;

  source = g_object_new (HUD_TYPE_RANDOM_SOURCE, NULL);

  source->max_depth = max_depth;
  source->max_items = max_items;
  source->max_words = max_words;
  source->max_letters = max_letters;

  hud_random_source_populate_table (source, rand, NULL, 0);

  return HUD_SOURCE (source);
}
