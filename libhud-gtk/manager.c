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
 * Author: Ted Gould <ted@canonical.com>
 */

#include "manager.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

struct _HudGtkManagerPrivate {
	GtkApplication * app;
};

#define HUD_GTK_MANAGER_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), HUD_GTK_TYPE_MANAGER, HudGtkManagerPrivate))

static void hud_gtk_manager_class_init (HudGtkManagerClass *klass);
static void hud_gtk_manager_init       (HudGtkManager *self);
static void hud_gtk_manager_dispose    (GObject *object);
static void hud_gtk_manager_finalize   (GObject *object);

G_DEFINE_TYPE (HudGtkManager, hud_gtk_manager, HUD_TYPE_MANAGER);

/* Initialize Class */
static void
hud_gtk_manager_class_init (HudGtkManagerClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (HudGtkManagerPrivate));

	object_class->dispose = hud_gtk_manager_dispose;
	object_class->finalize = hud_gtk_manager_finalize;

	return;
}

/* Initialize Instance */
static void
hud_gtk_manager_init (HudGtkManager *self)
{

	return;
}

/* Clean up refs */
static void
hud_gtk_manager_dispose (GObject *object)
{
	HudGtkManager * self = HUD_GTK_MANAGER(object);

	g_clear_object(&self->priv->app);

	G_OBJECT_CLASS (hud_gtk_manager_parent_class)->dispose (object);
	return;
}

/* Free Memory */
static void
hud_gtk_manager_finalize (GObject *object)
{

	G_OBJECT_CLASS (hud_gtk_manager_parent_class)->finalize (object);
	return;
}

/**
 * hud_gtk_manager_new:
 * @app: A #GtkApplication object
 *
 * Creates a #HudGtkManager object that is connected to the @app so that
 * new windows get tracked and their actions automatically added to so
 * the HUD can access them.
 *
 * Return value: (transfer full): A new #HudGtkManager object
 */
HudGtkManager *
hud_gtk_manager_new (GtkApplication * app)
{

	return NULL;
}
