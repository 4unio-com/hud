/*
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

#include "huditem.h"
#include "hudstringlist.h"

static void
test_hud_item_get_command_null_tokens (void)
{
  HudItem *item = hud_item_new (NULL, NULL, NULL, NULL, NULL, NULL, TRUE);
  g_assert_cmpstr(hud_item_get_command(item), ==, "No Command");

  g_object_unref (item);
}

static void
test_hud_item_get_context_null_tokens (void)
{
  HudItem *item = hud_item_new (NULL, NULL, NULL, NULL, NULL, NULL, TRUE);
  g_assert_cmpstr(hud_item_get_description(item), ==, "");

  g_object_unref (item);
}

static void
test_hud_item_get_context_null_tokens_tail (void)
{
  HudStringList *tokens;
  tokens = hud_string_list_add_item ("a", NULL);

  HudItem *item = hud_item_new (tokens, NULL, NULL, NULL, NULL, NULL, TRUE);
  g_assert_cmpstr(hud_item_get_description(item), ==, "");

  g_object_unref (item);
}

static void
test_hud_item_getters (void)
{
	HudItem *item;
	HudStringList *tokens, *keywords;
	guint token_len;
	HudToken **list;

	tokens = hud_string_list_add_item ("tokens1", NULL);
	tokens = hud_string_list_add_item ("tokens2", tokens);
	tokens = hud_string_list_add_item ("tokens3", tokens);
	keywords = hud_string_list_add_item ("keywords1", NULL);
	keywords = hud_string_list_add_item ("keywords2", keywords);
	keywords = hud_string_list_add_item ("keywords3", keywords);

	item = hud_item_new (tokens, keywords, "shortcut", "desktop_file", "app_icon", "description", TRUE);

	g_assert_cmpstr(hud_item_get_app_icon(item), ==, "app_icon");
	g_assert_cmpstr(hud_item_get_command(item), ==, "tokens3");
	g_assert_cmpstr(hud_item_get_description(item), ==, "description");
	g_assert_cmpint(hud_item_get_enabled(item), ==, TRUE);
	g_assert_cmpstr(hud_item_get_item_icon(item), ==, ""); // For the base class this is always ""
	g_assert(hud_item_get_keywords(item) == keywords);
	g_assert_cmpstr(hud_item_get_shortcut(item), ==, "shortcut");
	g_assert(hud_item_get_tokens(item) == tokens);
	g_assert_cmpuint(hud_item_get_usage(item), ==, 0); // It hasn't been used yet

	HudTokenList *token_list = hud_item_get_token_list (item);
  g_assert_cmpint(hud_token_list_get_length (token_list), ==, 6);
  list = hud_token_list_get_tokens (token_list);
  g_assert_cmpstr(hud_token_get_original(list[0], &token_len), ==, "tokens1");
  g_assert_cmpstr(hud_token_get_original(list[1], &token_len), ==, "tokens2");
  g_assert_cmpstr(hud_token_get_original(list[2], &token_len), ==, "tokens3");
  g_assert_cmpstr(hud_token_get_original(list[3], &token_len), ==, "keywords1");
  g_assert_cmpstr(hud_token_get_original(list[4], &token_len), ==, "keywords2");
  g_assert_cmpstr(hud_token_get_original(list[5], &token_len), ==, "keywords3");

	g_object_unref (item);
	hud_string_list_unref (tokens);
	hud_string_list_unref (keywords);
}

static void
test_hud_item_suite (void)
{
  g_test_add_func ("/hud/huditem/getters", test_hud_item_getters);
  g_test_add_func ("/hud/huditem/get_command_null_tokens", test_hud_item_get_command_null_tokens);
  g_test_add_func ("/hud/huditem/get_context_null_tokens", test_hud_item_get_context_null_tokens);
  g_test_add_func ("/hud/huditem/get_context_null_tokens_tail", test_hud_item_get_context_null_tokens_tail);
}

gint
main (gint argc, gchar * argv[])
{
#ifndef GLIB_VERSION_2_36
  g_type_init ();
#endif

	g_test_init(&argc, &argv, NULL);

	/* Test suites */
	test_hud_item_suite();

	return g_test_run ();
}
