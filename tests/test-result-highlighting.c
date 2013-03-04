/*
Test code for highlighting functions

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

#include <string.h>

#include "hudstringlist.h"
#include "hudresult.h"
#include "hudsettings.h"

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

static void
test_result_highlighting_base (void)
{
	HudItem *item;
	HudStringList *item_tokens;
	HudTokenList *search_tokens;
	
	item_tokens = hud_string_list_add_item ("foo", NULL);
	item_tokens = hud_string_list_add_item ("bar", item_tokens);
	item_tokens = hud_string_list_add_item ("mango", item_tokens);
	
	search_tokens = hud_token_list_new_from_string ("bar");
	
	item = hud_item_new (item_tokens, NULL, NULL, NULL, NULL, NULL, TRUE);

	HudResult *result = hud_result_new (item, search_tokens, 0);
	
	g_assert_cmpstr (hud_result_get_html_description (result), ==, "foo &gt; <b>bar</b> &gt; mango");
	
	hud_token_list_free (search_tokens);
	g_object_unref (result);
	g_object_unref (item);
	hud_string_list_unref (item_tokens);
	
	return;
}

static void
test_result_highlighting_baseutf8 (void)
{
	HudItem *item;
	HudStringList *item_tokens;
	HudTokenList *search_tokens;
	
	item_tokens = hud_string_list_add_item ("foo", NULL);
	item_tokens = hud_string_list_add_item ("ẃêỳᶉ∂", item_tokens);
	item_tokens = hud_string_list_add_item ("mango", item_tokens);
	
	search_tokens = hud_token_list_new_from_string ("ẃêỳᶉ∂");
	
	item = hud_item_new (item_tokens, NULL, NULL, NULL, NULL, NULL, TRUE);

	HudResult *result = hud_result_new (item, search_tokens, 0);
	
	g_assert_cmpstr (hud_result_get_html_description (result), ==, "foo &gt; <b>ẃêỳᶉ∂</b> &gt; mango");
	
	hud_token_list_free (search_tokens);
	g_object_unref (result);
	g_object_unref (item);
	hud_string_list_unref (item_tokens);
	
	return;
}

static void
test_result_highlighting_extra_keywords (void)
{
  HudItem *item;
  HudStringList *item_tokens, *item_keywords;
  HudTokenList *search_tokens;

  item_tokens = hud_string_list_add_item ("File", NULL);
  item_tokens = hud_string_list_add_item ("Open Tab", item_tokens);

  item_keywords = hud_string_list_add_item ("Gimme a Tab Bro", NULL);
  item_keywords = hud_string_list_add_item ("Giv Tab Plz", item_keywords);

  search_tokens = hud_token_list_new_from_string ("plz");

  item = hud_item_new (item_tokens, item_keywords, NULL, NULL, NULL, NULL, TRUE);

  HudResult *result = hud_result_new (item, search_tokens, 0);
  g_assert_cmpstr (hud_result_get_html_description (result), ==, "File &gt; Open Tab (Giv Tab <b>Plz</b>)");

  hud_token_list_free (search_tokens);
  g_object_unref (result);
  g_object_unref (item);
  hud_string_list_unref (item_tokens);
  hud_string_list_unref (item_keywords);

  return;
}

static void
test_result_highlighting_extra_keywords_multiple_hits (void)
{
  HudItem *item;
  HudStringList *item_tokens, *item_keywords;
  HudTokenList *search_tokens;

  item_tokens = hud_string_list_add_item ("File", NULL);
  item_tokens = hud_string_list_add_item ("Open Tab", item_tokens);

  item_keywords = hud_string_list_add_item ("Gimme a Tab Bro", NULL);
  item_keywords = hud_string_list_add_item ("Giv Tab Plz", item_keywords);

  search_tokens = hud_token_list_new_from_string ("bro plz");

  item = hud_item_new (item_tokens, item_keywords, NULL, NULL, NULL, NULL, TRUE);

  HudResult *result = hud_result_new (item, search_tokens, 0);
  g_assert_cmpstr(hud_result_get_html_description (result), ==, "File &gt; Open Tab (Gimme a Tab <b>Bro</b>; Giv Tab <b>Plz</b>)");

  hud_token_list_free (search_tokens);
  g_object_unref (result);
  g_object_unref (item);
  hud_string_list_unref (item_tokens);
  hud_string_list_unref (item_keywords);

  return;
}

static void
test_result_highlighting_gt (void)
{
	HudItem *item;
	HudStringList *item_tokens;
	HudTokenList *search_tokens;
	
	item_tokens = hud_string_list_add_item ("foo", NULL);
	item_tokens = hud_string_list_add_item ("bar", item_tokens);
	item_tokens = hud_string_list_add_item ("gt", item_tokens);
	
	search_tokens = hud_token_list_new_from_string ("gt");
	
	item = hud_item_new (item_tokens, NULL, NULL, NULL, NULL, NULL, TRUE);

	HudResult *result = hud_result_new (item, search_tokens, 0);
	g_assert_cmpstr (hud_result_get_html_description (result), ==, "foo &gt; bar &gt; <b>gt</b>");
	
	hud_token_list_free (search_tokens);
	g_object_unref (result);
	g_object_unref (item);
	hud_string_list_unref (item_tokens);
	
	return;
}

static void
test_result_highlighting_apos1 (void)
{
	HudItem *item;
	HudStringList *item_tokens;
	HudTokenList *search_tokens;

	item_tokens = hud_string_list_add_item ("d'interes", NULL);
	item_tokens = hud_string_list_add_item ("a", item_tokens);

	search_tokens = hud_token_list_new_from_string ("d'in");

	item = hud_item_new (item_tokens, NULL, NULL, NULL, NULL, NULL, TRUE);

	HudResult *result = hud_result_new (item, search_tokens, 0);
	g_assert_cmpstr (hud_result_get_html_description (result), ==, "<b>d&apos;interes</b> &gt; a");

	hud_token_list_free (search_tokens);
	g_object_unref (result);
	g_object_unref (item);
	hud_string_list_unref (item_tokens);

	return;
}

static void
test_result_highlighting_apos2 (void)
{
	HudItem *item;
	HudStringList *item_tokens;
	HudTokenList *search_tokens;

	item_tokens = hud_string_list_add_item ("d'interes", NULL);
	item_tokens = hud_string_list_add_item ("a", item_tokens);

	search_tokens = hud_token_list_new_from_string ("a");

	item = hud_item_new (item_tokens, NULL, NULL, NULL, NULL, NULL, TRUE);

	HudResult *result = hud_result_new (item, search_tokens, 0);
	g_assert_cmpstr (hud_result_get_html_description (result), ==, "d&apos;interes &gt; <b>a</b>");

	hud_token_list_free (search_tokens);
	g_object_unref (result);
	g_object_unref (item);
	hud_string_list_unref (item_tokens);

	return;
}

/* Build the test suite */
static void
test_result_highlighting_suite (void)
{
	g_test_add_func ("/hud/highlighting/base",          test_result_highlighting_base);
	g_test_add_func ("/hud/highlighting/baseutf8",      test_result_highlighting_baseutf8);
	g_test_add_func ("/hud/highlighting/extra_keywords",test_result_highlighting_extra_keywords);
	g_test_add_func ("/hud/highlighting/extra_keywords_multiple_hits",test_result_highlighting_extra_keywords_multiple_hits);
	g_test_add_func ("/hud/highlighting/gt",            test_result_highlighting_gt);
	g_test_add_func ("/hud/highlighting/apos1",         test_result_highlighting_apos1);
	g_test_add_func ("/hud/highlighting/apos2",         test_result_highlighting_apos2);
	return;
}

gint
main (gint argc, gchar * argv[])
{
#ifndef GLIB_VERSION_2_36
	g_type_init ();
#endif

	g_test_init(&argc, &argv, NULL);

	/* Test suites */
	test_result_highlighting_suite();

	return g_test_run ();
}
