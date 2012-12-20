/*
 Test code for string list

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

#include <glib-object.h>

#include "hudstringlist.h"

static void
test_hud_string_list_cons ()
{
  HudStringList *list = NULL, *first = NULL, *second = NULL, *third = NULL,
      *tmp = NULL;

  first = hud_string_list_cons ("first", NULL );
  list = first;
  g_assert_cmpstr(hud_string_list_get_head(list), ==, "first");
  g_assert(hud_string_list_get_tail(list) == NULL);

  second = hud_string_list_cons ("second", list);
  hud_string_list_unref (list);
  list = second;
  g_assert_cmpstr(hud_string_list_get_head(list), ==, "second");
  g_assert(hud_string_list_get_tail(list) == first);

  tmp = hud_string_list_get_tail (list);
  g_assert_cmpstr(hud_string_list_get_head(tmp), ==, "first");
  g_assert(hud_string_list_get_tail(tmp) == NULL);

  third = hud_string_list_cons ("third", list);
  hud_string_list_unref (list);
  list = third;
  g_assert_cmpstr(hud_string_list_get_head(list), ==, "third");
  g_assert(hud_string_list_get_tail(list) == second);

  tmp = hud_string_list_get_tail (list);
  g_assert_cmpstr(hud_string_list_get_head(tmp), ==, "second");
  g_assert(hud_string_list_get_tail(tmp) == first);

  tmp = hud_string_list_get_tail (tmp);
  g_assert_cmpstr(hud_string_list_get_head(tmp), ==, "first");
  g_assert(hud_string_list_get_tail(tmp) == NULL);

  hud_string_list_unref (list);
}

static void
test_hud_string_list_cons_label ()
{
  HudStringList *list = NULL, *first = NULL, *second = NULL, *third = NULL,
      *tmp = NULL;

  first = hud_string_list_cons_label ("f_irs_t", NULL );
  list = first;
  g_assert_cmpstr(hud_string_list_get_head(list), ==, "first");
  g_assert(hud_string_list_get_tail(list) == NULL);

  second = hud_string_list_cons_label ("_sec_ond", list);
  hud_string_list_unref (list);
  list = second;
  g_assert_cmpstr(hud_string_list_get_head(list), ==, "second");
  g_assert(hud_string_list_get_tail(list) == first);

  tmp = hud_string_list_get_tail (list);
  g_assert_cmpstr(hud_string_list_get_head(tmp), ==, "first");
  g_assert(hud_string_list_get_tail(tmp) == NULL);

  third = hud_string_list_cons_label ("third_", list);
  hud_string_list_unref (list);
  list = third;
  /* Note that it's not supposed to strip the underscore from the end */
  g_assert_cmpstr(hud_string_list_get_head(list), ==, "third_");
  g_assert(hud_string_list_get_tail(list) == second);

  tmp = hud_string_list_get_tail (list);
  g_assert_cmpstr(hud_string_list_get_head(tmp), ==, "second");
  g_assert(hud_string_list_get_tail(tmp) == first);

  tmp = hud_string_list_get_tail (tmp);
  g_assert_cmpstr(hud_string_list_get_head(tmp), ==, "first");
  g_assert(hud_string_list_get_tail(tmp) == NULL);

  hud_string_list_unref (list);
}

static void
test_hud_string_list_pretty_print ()
{
  HudStringList *list = NULL, *tmp = NULL;

  list = hud_string_list_cons ("first", NULL );

  tmp = hud_string_list_cons ("second", list);
  hud_string_list_unref (list);
  list = tmp;

  tmp = hud_string_list_cons ("third", list);
  hud_string_list_unref (list);
  list = tmp;

  tmp = hud_string_list_cons ("fourth", list);
  hud_string_list_unref (list);
  list = tmp;

  g_assert_cmpstr(hud_string_list_pretty_print(list), ==,
      "first > second > third > fourth");

  hud_string_list_unref (list);
}

static void
test_hud_string_list_pretty_print_null ()
{
  g_assert_cmpstr(hud_string_list_pretty_print(NULL), ==, "");
}

static void
test_string_list_suite ()
{
  g_test_add_func ("/hud/string_list/cons", test_hud_string_list_cons);
  g_test_add_func ("/hud/string_list/cons_label",
      test_hud_string_list_cons_label);
  g_test_add_func ("/hud/string_list/pretty_print",
      test_hud_string_list_pretty_print);
  g_test_add_func ("/hud/string_list/pretty_print_null",
      test_hud_string_list_pretty_print_null);
}

gint
main (gint argc, gchar * argv[])
{
  g_type_init ();
  g_test_init (&argc, &argv, NULL );
  test_string_list_suite ();
  return g_test_run ();
}
