/*
 * Copyright © 2013 Canonical Ltd.
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

#include "application-source-context.h"

static void hud_application_source_context_class_init (HudApplicationSourceContextClass *klass);
static void hud_application_source_context_init       (HudApplicationSourceContext *self);
static void hud_application_source_context_dispose    (GObject *object);
static void hud_application_source_context_finalize   (GObject *object);

G_DEFINE_TYPE (HudApplicationSourceContext, hud_application_source_context, G_TYPE_OBJECT);

static void
hud_application_source_context_class_init (HudApplicationSourceContextClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->dispose = hud_application_source_context_dispose;
	object_class->finalize = hud_application_source_context_finalize;

	return;
}

static void
hud_application_source_context_init (HudApplicationSourceContext *self)
{

	return;
}

static void
hud_application_source_context_dispose (GObject *object)
{

	G_OBJECT_CLASS (hud_application_source_context_parent_class)->dispose (object);
	return;
}

static void
hud_application_source_context_finalize (GObject *object)
{

	G_OBJECT_CLASS (hud_application_source_context_parent_class)->finalize (object);
	return;
}
