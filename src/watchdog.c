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
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "watchdog.h"

typedef struct _HudWatchdogPrivate HudWatchdogPrivate;
struct _HudWatchdogPrivate {
	gulong timer;
};

#define HUD_WATCHDOG_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), HUD_WATCHDOG_TYPE, HudWatchdogPrivate))

static void hud_watchdog_class_init (HudWatchdogClass *klass);
static void hud_watchdog_init       (HudWatchdog *self);
static void hud_watchdog_dispose    (GObject *object);
static void hud_watchdog_finalize   (GObject *object);

G_DEFINE_TYPE (HudWatchdog, hud_watchdog, G_TYPE_OBJECT);

static void
hud_watchdog_class_init (HudWatchdogClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (HudWatchdogPrivate));

	object_class->dispose = hud_watchdog_dispose;
	object_class->finalize = hud_watchdog_finalize;

	return;
}

static void
hud_watchdog_init (HudWatchdog *self)
{

	return;
}

static void
hud_watchdog_dispose (GObject *object)
{

	G_OBJECT_CLASS (hud_watchdog_parent_class)->dispose (object);
	return;
}

static void
hud_watchdog_finalize (GObject *object)
{

	G_OBJECT_CLASS (hud_watchdog_parent_class)->finalize (object);
	return;
}
