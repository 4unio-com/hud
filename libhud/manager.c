/*
 * Copyright © 2012 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of either or both of the following licences:
 *
 * 1) the GNU Lesser General Public License version 3, as published by
 * the Free Software Foundation; and/or
 * 2) the GNU Lesser General Public License version 2.1, as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the applicable version of the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of both the GNU Lesser General Public
 * License version 3 and version 2.1 along with this program.  If not,
 * see <http://www.gnu.org/licenses/>
 *
 * Author: Ryan Lortie <desrt@desrt.ca>
 */

#include "manager.h"

void
hud_manager_say_hello (const gchar *application_id,
                       const gchar *description_path)
{
  g_message ("Application '%s' describes actions at '%s'", application_id, description_path);
}

void
hud_manager_say_goodbye (const gchar *application_id)
{
  g_message ("Application '%s' is removing its action descriptions", application_id);
}

void
hud_manager_add_actions (const gchar *application_id,
                         const gchar *prefix,
                         GVariant    *identifier,
                         const gchar *object_path)
{
  gchar *identifier_str;

  if (identifier)
    {
      identifier_str = g_variant_print (g_variant_ref_sink (identifier), TRUE);
      g_variant_unref (identifier);
    }
  else
    identifier_str = g_strdup ("(nil)");

  g_message ("Application '%s' has '%s' (identifier '%s') actions at '%s'",
             application_id, prefix, identifier_str, object_path);

  g_free (identifier_str);
}

void
hud_manager_remove_actions (const gchar *application_id,
                            const gchar *prefix,
                            GVariant    *identifier)
{
  gchar *identifier_str;

  if (identifier)
    {
      identifier_str = g_variant_print (g_variant_ref_sink (identifier), TRUE);
      g_variant_unref (identifier);
    }
  else
    identifier_str = g_strdup ("(nil)");

  g_message ("Application '%s' drops '%s' (identifier '%s') actions",
             application_id, prefix, identifier_str);

  g_free (identifier_str);
}
