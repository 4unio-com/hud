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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

struct _HudManagerPrivate {
	int dummy;
};

#define HUD_MANAGER_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), HUD_TYPE_MANAGER, HudManagerPrivate))

static void hud_manager_class_init (HudManagerClass *klass);
static void hud_manager_init       (HudManager *self);
static void hud_manager_dispose    (GObject *object);
static void hud_manager_finalize   (GObject *object);

G_DEFINE_TYPE (HudManager, hud_manager, G_TYPE_OBJECT);

/* Initialize Class */
static void
hud_manager_class_init (HudManagerClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (HudManagerPrivate));

	object_class->dispose = hud_manager_dispose;
	object_class->finalize = hud_manager_finalize;

	return;
}

/* Initialize Instance */
static void
hud_manager_init (HudManager *self)
{

	return;
}

/* Clean up refs */
static void
hud_manager_dispose (GObject *object)
{

	G_OBJECT_CLASS (hud_manager_parent_class)->dispose (object);
	return;
}

/* Free Memory */
static void
hud_manager_finalize (GObject *object)
{

	G_OBJECT_CLASS (hud_manager_parent_class)->finalize (object);
	return;
}

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
