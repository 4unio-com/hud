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
#include <libxml/parser.h>

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

static gchar*
get_language ()
{
  return getenv ("LANGUAGE");
}

static void
set_language (const gchar* language)
{
  /* Change language.  */
  setenv ("LANGUAGE", language, 1);

  /* Make change known.  */
  {
    extern int _nl_msg_cat_cntr;
    ++_nl_msg_cat_cntr;
  }
}

static void
test_keyword_mapping_open_tab (void)
{
  HudKeywordMapping *mapping;
  GPtrArray* results;

  mapping = hud_keyword_mapping_new ();
  hud_keyword_mapping_load (mapping, "/foo/bar/gnome-terminal.desktop",
      "keyword-mapping", "./keyword-mapping/locale");
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

  gchar* original_language = get_language ();
  mapping = hud_keyword_mapping_new ();

  /* Temporarily change to language to read in our translations */
  set_language ("en_FAKELANG");
  hud_keyword_mapping_load (mapping, "/foo/bar/gnome-terminal.desktop",
      "keyword-mapping", "./keyword-mapping/locale");
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
      "./keyword-mapping/locale");
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
  setlocale (LC_ALL, "");

  /* Init libxml */
  xmlInitParser ();
  LIBXML_TEST_VERSION

  g_type_init ();

  g_test_init (&argc, &argv, NULL );

  /* Test suites */
  test_keyword_mapping_suite ();

  gint result = g_test_run ();

  /* Shutdown libxml */
  xmlCleanupParser ();

  return result;
}
