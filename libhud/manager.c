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

/**
 * hud_manager_new:
 * @application_id: Unique identifier of the application, usually desktop file name
 *
 * Creates a new #HudManager object.
 *
 * Return value: (transfer full): New #HudManager
 */
HudManager *
hud_manager_new (const gchar * application_id)
{
	g_return_val_if_fail(application_id != NULL, NULL);

	return g_object_new(HUD_TYPE_MANAGER, NULL);
}

/**
 * hud_manager_add_actions:
 * @manager: A #HudManager object
 * @id: (allow none): Identifier of the user object where these actions are
 * @pub: Action publisher object tracking the descriptions and action groups
 *
 * Sets up a set of actions and descriptions for a specific user
 * context.  This could be a window or a tab, depending on how the
 * application works.
 */
void
hud_manager_add_actions (HudManager * manager, GVariant * id, HudActionPublisher * pub)
{
	g_return_if_fail(HUD_IS_MANAGER(manager));

	return;
}

/**
 * hud_manager_remove_actions:
 * @manager: A #HudManager object
 * @pub: Action publisher object tracking the descriptions and action groups
 *
 * Removes actions for being watched by the HUD.  Should be done when the object
 * is remove.  Does not require @pub to be a valid object so it can be used
 * with weak pointer style destroy.
 */
void
hud_manager_remove_actions (HudManager * manager, HudActionPublisher * pub)
{
	g_return_if_fail(HUD_IS_MANAGER(manager));

	return;
}
