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

#include "pronounce-dict.h"

static void
test_hud_pronouce_dict_hashes ()
{
  PronounceDict *dict = pronounce_dict_new("test-pronounce-dict-hashes.dic");

  {
    gchar** pronounciations = pronounce_dict_lookup_word (dict, "hello");
    g_assert_cmpint(g_strv_length(pronounciations), ==, 1);
    g_assert_cmpstr(pronounciations[0], ==, "hh ax l ow");
    g_strfreev(pronounciations);
  }

  {
    gchar** pronounciations = pronounce_dict_lookup_word (dict, "there");
    g_assert_cmpint(g_strv_length(pronounciations), ==, 2);
    g_assert_cmpstr(pronounciations[0], ==, "dh ea");
    g_assert_cmpstr(pronounciations[1], ==, "dh ea r");
    g_strfreev(pronounciations);
  }

  g_object_unref(dict);
}

static void
test_hud_pronouce_dict_semicolon ()
{
  PronounceDict *dict = pronounce_dict_new("test-pronounce-dict-semicolon.dic");

  {
    gchar** pronounciations = pronounce_dict_lookup_word (dict, "hello");
    g_assert_cmpint(g_strv_length(pronounciations), ==, 2);
    g_assert_cmpstr(pronounciations[0], ==, "HH AH L OW");
    g_assert_cmpstr(pronounciations[1], ==, "HH EH L OW");
    g_strfreev(pronounciations);
  }

  {
    gchar** pronounciations = pronounce_dict_lookup_word (dict, "there");
    g_assert_cmpint(g_strv_length(pronounciations), ==, 1);
    g_assert_cmpstr(pronounciations[0], ==, "DH EH R");
    g_strfreev(pronounciations);
  }

  g_object_unref(dict);
}

static void
test_hud_pronouce_dict_lowercase ()
{
  PronounceDict *dict = pronounce_dict_new("test-pronounce-dict-lowercase.dic");

  {
    gchar** pronounciations = pronounce_dict_lookup_word (dict, "hello");
    g_assert_cmpint(g_strv_length(pronounciations), ==, 2);
    g_assert_cmpstr(pronounciations[0], ==, "HH AH L OW");
    g_assert_cmpstr(pronounciations[1], ==, "HH EH L OW");
    g_strfreev(pronounciations);
  }

  {
    gchar** pronounciations = pronounce_dict_lookup_word (dict, "there");
    g_assert_cmpint(g_strv_length(pronounciations), ==, 1);
    g_assert_cmpstr(pronounciations[0], ==, "DH EH R");
    g_strfreev(pronounciations);
  }

  g_object_unref(dict);
}

static void
test_hud_pronouce_dict_htk ()
{
  PronounceDict *dict = pronounce_dict_new("test-pronounce-dict-htk.dic");

  {
    gchar** pronounciations = pronounce_dict_lookup_word (dict, "abandon");
    g_assert_cmpint(g_strv_length(pronounciations), ==, 1);
    g_assert_cmpstr(pronounciations[0], ==, "ax b ae n d ax n");
    g_strfreev(pronounciations);
  }

  {
    gchar** pronounciations = pronounce_dict_lookup_word (dict, "abandoned");
    g_assert_cmpint(g_strv_length(pronounciations), ==, 1);
    g_assert_cmpstr(pronounciations[0], ==, "ax b ae n d ax n d");
    g_strfreev(pronounciations);
  }

  {
    gchar** pronounciations = pronounce_dict_lookup_word (dict, "abandonment");
    g_assert_cmpint(g_strv_length(pronounciations), ==, 1);
    g_assert_cmpstr(pronounciations[0], ==, "ax b ae n d ax n m ax n t");
    g_strfreev(pronounciations);
  }

  g_object_unref(dict);
}

static void
test_string_list_suite ()
{
  g_test_add_func ("/hud/pronounce_dict/hashes", test_hud_pronouce_dict_hashes);
  g_test_add_func ("/hud/pronounce_dict/semicolon", test_hud_pronouce_dict_semicolon);
  g_test_add_func ("/hud/pronounce_dict/lowercase", test_hud_pronouce_dict_lowercase);
  g_test_add_func ("/hud/pronounce_dict/htk", test_hud_pronouce_dict_htk);
}

gint
main (gint argc, gchar * argv[])
{
#ifndef GLIB_VERSION_2_36
  g_type_init ();
#endif
  g_test_init (&argc, &argv, NULL );
  test_string_list_suite ();
  return g_test_run ();
}
