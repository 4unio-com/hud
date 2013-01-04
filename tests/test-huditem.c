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

static void
test_result_highlighting_baseutf8 (void)
{
	HudItem *item;
	HudStringList *item_tokens;
	HudTokenList *search_tokens;

	item_tokens = add_item_to_hud_string_list ("foo", NULL);
	item_tokens = add_item_to_hud_string_list ("ẃêỳᶉ∂", item_tokens);
	item_tokens = add_item_to_hud_string_list ("mango", item_tokens);

	search_tokens = hud_token_list_new_from_string ("ẃêỳᶉ∂");

	item = hud_item_new (item_tokens, NULL, NULL, NULL, NULL, TRUE);

	HudResult *result = hud_result_new (item, search_tokens, 0);

	g_assert_cmpstr (hud_result_get_html_description (result), ==, "foo &gt; <b>ẃêỳᶉ∂</b> &gt; mango");

	hud_token_list_free (search_tokens);
	g_object_unref (result);
	g_object_unref (item);
	hud_string_list_unref (item_tokens);

	return;
}

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
	//gtk_init(&argc, &argv);
	g_type_init();

	g_test_init(&argc, &argv, NULL);

	/* Test suites */
	test_result_highlighting_suite();

	return g_test_run ();
}
