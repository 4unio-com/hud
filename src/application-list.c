/*
 * Copyright © 2012 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Ted Gould <ted@canonical.com>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "application-list.h"

typedef struct _HudApplicationListPrivate HudApplicationListPrivate;

struct _HudApplicationListPrivate {
	int dummy;
};

#define HUD_APPLICATION_LIST_GET_PRIVATE(o) \
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), HUD_APPLICATION_LIST_TYPE, HudApplicationListPrivate))

static void hud_application_list_class_init (HudApplicationListClass *klass);
static void hud_application_list_init       (HudApplicationList *self);
static void hud_application_list_dispose    (GObject *object);
static void hud_application_list_finalize   (GObject *object);

G_DEFINE_TYPE (HudApplicationList, hud_application_list, G_TYPE_OBJECT);

/* Class Init */
static void
hud_application_list_class_init (HudApplicationListClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (HudApplicationListPrivate));

	object_class->dispose = hud_application_list_dispose;
	object_class->finalize = hud_application_list_finalize;

	return;
}

/* Instance Init */
static void
hud_application_list_init (HudApplicationList *self)
{

	return;
}

/* Clean up references */
static void
hud_application_list_dispose (GObject *object)
{

	G_OBJECT_CLASS (hud_application_list_parent_class)->dispose (object);
	return;
}

/* Free memory */
static void
hud_application_list_finalize (GObject *object)
{

	G_OBJECT_CLASS (hud_application_list_parent_class)->finalize (object);
	return;
}

