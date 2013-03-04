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
 * Author: Ryan Lortie <desrt@desrt.ca>
 */

#include "hudsettings.h"
#include "hudquery.h"
#include "hudtoken.h"
#include "hudsource.h"
#include "hudrandomsource.h"

#include <glib-object.h>
#include <gio/gio.h>

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

void
test_query_performance (void)
{
  HudSource *source;
  HudQuery *query;
  GRand *rand;
  gint i;

  rand = g_rand_new_with_seed (1234);
  source = hud_random_source_new (rand);

  for (i = 1; i <= 6; i++)
    {
      guint64 start_time;
      gchar *search;
      gint j;

      g_print ("\n");

      search = hud_random_source_make_words (rand, i);

      /* simulate the user typing it in, one character at a time */
      for (j = 1; search[j - 1]; j++)
        {
          gchar *part_search = g_strndup (search, j);

          start_time = g_get_monotonic_time ();
          query = hud_query_new (source, source, part_search, 1u<<30, g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, NULL), 0);
          g_print ("%-60s: %dus\n", part_search,
                   (int) (g_get_monotonic_time () - start_time));
          g_object_unref (query);
          g_free (part_search);
        }

      g_free (search);
    }

  g_object_unref (source);
  g_rand_free (rand);
}

int
main (int argc, char **argv)
{
#ifndef GLIB_VERSION_2_36
  g_type_init();
#endif

  g_test_init (&argc, &argv, NULL);
  if (g_test_perf ())
    g_test_add_func ("/hud/query-performance", test_query_performance);

  return g_test_run ();
}
