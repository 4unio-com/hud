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

#include "hud-collector.h"

static void hud_collector_class_init (HudCollectorClass *klass);
static void hud_collector_init       (HudCollector *self);

G_DEFINE_TYPE (HudCollector, hud_collector, G_TYPE_OBJECT);

static void
hud_collector_class_init (HudCollectorClass *klass)
{
	return;
}

static void
hud_collector_init (HudCollector *self)
{
	return;
}

GList *
hud_collector_get_items (HudCollector * collector)
{
	g_return_val_if_fail(HUD_IS_COLLECTOR(collector), NULL);

	HudCollectorClass * klass = HUD_COLLECTOR_GET_CLASS(collector);
	if (klass->get_items != NULL) {
		return klass->get_items(collector);
	}

	return NULL;
}
