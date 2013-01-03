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

#define G_LOG_DOMAIN "test-source"

#include "hudsettings.h"
#include "hudquery.h"
#include "hudtoken.h"
#include "hudsource.h"
#include "hudsourcelist.h"
#include "hudrandomsource.h"

#include <glib-object.h>
#include <dee.h>

/* hardcode some parameters for reasons of determinism.
 */
HudSettings hud_settings = {
  .indicator_penalty = 50,
  .add_penalty = 10,
  .drop_penalty = 10,
  .end_drop_penalty = 1,
  .swap_penalty = 15,
  .max_distance = 30
};

static void
make_assertion (HudSource *source, const gchar *search,
    const gchar *appstack, const gchar *path, const gchar *name,
    const gchar **expected_rows, const guint32 *expected_distances,
    const guint expected_count)
{
  HudQuery *query;
  guint row;

  g_debug ("query: %s", search);

  query = hud_query_new (source, search, 1u << 30);

  g_assert_cmpstr(hud_query_get_appstack_name(query), ==, appstack);
  g_assert_cmpstr(hud_query_get_path(query), ==, path);
  g_assert_cmpstr(hud_query_get_results_name(query), ==, name);

  DeeModel *model = hud_query_get_results_model(query);

  g_assert_cmpint(dee_model_get_n_rows(model), ==, expected_count);

  for(row = 0; row < expected_count ; row++)
  {
    DeeModelIter* iter = dee_model_get_iter_at_row(model, row);

    g_debug("Result: %s", dee_model_get_string(model, iter, 1));
    g_debug("Expected: %s", expected_rows[row]);
    g_debug("Distance: %d", dee_model_get_uint32(model, iter, 6));
    g_debug("Exp Distance: %d", expected_distances[row]);

    g_assert_cmpstr(dee_model_get_string(model, iter, 1), ==, expected_rows[row]);
    g_assert_cmpint(dee_model_get_uint32(model, iter, 6), ==, expected_distances[row]);
  }

  g_object_unref (query);
}

static void
test_hud_query (void)
{
  HudSource *random_source;
  GRand *rand;

  rand = g_rand_new_with_seed (8);
  random_source = hud_random_source_new_full (rand, 1, 4, 2);

  HudSourceList *source_list = hud_source_list_new();
  hud_source_list_add(source_list, random_source);

  {
    gchar *search = "ash";
    const gchar *expected[5] = { "any rash", "stray slash", "swift sad", "itch step", "mess strand" };
    const guint32 expected_distances[5] = { 10, 20, 22, 23, 25 };
    const gchar *appstack = "com.canonical.hud.query0.appstack";
    const gchar *path = "/com/canonical/hud/query0";
    const gchar *name = "com.canonical.hud.query0.results";
    make_assertion (HUD_SOURCE(source_list), search, appstack, path, name, expected, expected_distances, 5);
  }

  {
    gchar *search = "mess";
    const gchar *expected[1] = { "mess strand"};
    const guint32 expected_distances[1] = { 1 };
    const gchar *appstack = "com.canonical.hud.query1.appstack";
    const gchar *path = "/com/canonical/hud/query1";
    const gchar *name = "com.canonical.hud.query1.results";
    make_assertion (HUD_SOURCE(source_list), search, appstack, path, name, expected, expected_distances, 1);
  }

  {
    gchar *search = "dare";
    const gchar *expected[1] = { "mess strand"};
    const guint32 expected_distances[1] = { 2 };
    const gchar *appstack = "com.canonical.hud.query2.appstack";
    const gchar *path = "/com/canonical/hud/query2";
    const gchar *name = "com.canonical.hud.query2.results";
    make_assertion (HUD_SOURCE(source_list), search, appstack, path, name, expected, expected_distances, 1);
  }

  g_object_unref (random_source);
  g_object_unref (source_list);
  g_rand_free (rand);
}

int
main (int argc, char **argv)
{
  g_type_init ();

  g_test_init (&argc, &argv, NULL);
  g_test_add_func ("/hud/source", test_hud_query);

  return g_test_run ();
}
