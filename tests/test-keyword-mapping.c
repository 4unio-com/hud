/*
Test code for keyword mapping

Copyright 2012 Canonical Ltd.

This program is free software: you can redistribute it and/or modify it 
under the terms of the GNU General Public License version 3, as published 
by the Free Software Foundation.

This program is distributed in the hope that it will be useful, but 
WITHOUT ANY WARRANTY; without even the implied warranties of 
MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR 
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along 
with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <glib.h>
#include <glib-object.h>
#include <glib/gi18n.h>
#include <locale.h>

#include <string.h>
#include <stdlib.h>

#include "hudsettings.h"
#include "hudkeywordmapping.h"

/* hardcode some parameters so the test doesn't fail if the user
 * has bogus things in GSettings.
 */
HudSettings hud_settings = {
	.indicator_penalty = 50,
	.add_penalty = 10,
	.drop_penalty = 10,
	.end_drop_penalty = 1,
	.swap_penalty = 15,
	.max_distance = 30
};

static
const gchar*
get_language ()
{
  return g_getenv ("LANGUAGE");
}

/**
 *
 */
static void
set_language (const gchar* language)
{
  /* Change language.  */
  g_setenv ("LANGUAGE", language, 1);

  /* Make change known.  */
  {
    extern int _nl_msg_cat_cntr;
    ++_nl_msg_cat_cntr;
  }
}

static void
test_keyword_mapping_unknown_action (void)
{
  HudKeywordMapping *mapping;
  GPtrArray *results, *results2;

  mapping = hud_keyword_mapping_new ();
  hud_keyword_mapping_load (mapping, "gnome-terminal",
      "keyword-mapping", KEYWORD_LOCALE_DIR);

  results = hud_keyword_mapping_transform (mapping, "random action string");
  g_assert_cmpint(results->len, ==, 0);

  results2 = hud_keyword_mapping_transform (mapping, "random action string");
  g_assert_cmpint(results2->len, ==, 0);

  /* Check the keyword miss was cached */
  g_assert(results == results2);

  g_object_unref (mapping);
}

static void
test_keyword_mapping_open_tab (void)
{
  HudKeywordMapping *mapping;
  GPtrArray* results;

  mapping = hud_keyword_mapping_new ();
  hud_keyword_mapping_load (mapping, "gnome-terminal",
      "keyword-mapping", KEYWORD_LOCALE_DIR);
  results = hud_keyword_mapping_transform (mapping, "Open Ta_b");

  g_assert_cmpint(results->len, ==, 2);
  g_assert_cmpstr((gchar*) g_ptr_array_index(results, 0), ==, "New Tab");
  g_assert_cmpstr((gchar*) g_ptr_array_index(results, 1), ==, "Another Tab");

  g_object_unref (mapping);
}

static void
test_keyword_mapping_open_tab_with_translation (void)
{
  HudKeywordMapping *mapping;
  GPtrArray* results;

  const gchar* original_language = get_language ();
  mapping = hud_keyword_mapping_new ();

  /* Temporarily change to language to read in our translations */
  set_language ("en_FAKELANG");
  hud_keyword_mapping_load (mapping, "gnome-terminal",
      "keyword-mapping", KEYWORD_LOCALE_DIR);
  set_language (original_language);

  results = hud_keyword_mapping_transform (mapping, "Open Ta_b");

  g_assert_cmpint(results->len, ==, 3);
  g_assert_cmpstr((gchar*) g_ptr_array_index(results, 0), ==, "More Tabs");
  g_assert_cmpstr((gchar*) g_ptr_array_index(results, 1), ==,
      "Gimme a Tab Bro");
  g_assert_cmpstr((gchar*) g_ptr_array_index(results, 2), ==, "Giv Tab Plz");

  g_object_unref (mapping);
}

static void
test_keyword_mapping_open_terminal (void)
{
  HudKeywordMapping *mapping;
  GPtrArray* results;

  mapping = hud_keyword_mapping_new ();
  hud_keyword_mapping_load (mapping, "gnome-terminal", "keyword-mapping",
      KEYWORD_LOCALE_DIR);
  results = hud_keyword_mapping_transform (mapping, "Open _Terminal");

  g_assert_cmpint(results->len, ==, 2);
  g_assert_cmpstr((gchar*) g_ptr_array_index(results, 0), ==, "New Terminal");
  g_assert_cmpstr((gchar*) g_ptr_array_index(results, 1), ==,
      "Another Terminal");

  g_object_unref (mapping);
}

/* Build the test suite */
static void test_keyword_mapping_suite (void)
{
  g_test_add_func ("/hud/keyword/unknown_action", test_keyword_mapping_unknown_action);
  g_test_add_func ("/hud/keyword/open_tab", test_keyword_mapping_open_tab);
  g_test_add_func ("/hud/keyword/open_tab_with_translation",
      test_keyword_mapping_open_tab_with_translation);
  g_test_add_func ("/hud/keyword/open_terminal",
      test_keyword_mapping_open_terminal);
  return;
}

gint
main (gint argc, gchar * argv[])
{
  g_setenv ("LANG", "C.UTF-8", 1);
  g_unsetenv("LC_ALL");
  setlocale (LC_ALL, "");

#ifndef GLIB_VERSION_2_36
  g_type_init();
#endif

  g_test_init (&argc, &argv, NULL );

  /* Test suites */
  test_keyword_mapping_suite ();

  gint result = g_test_run ();

  return result;
}
