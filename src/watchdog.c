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

#include <stdlib.h>
#include "watchdog.h"

#define DEFAULT_TIMEOUT  600 /* seconds, or 10 minutes */

typedef struct _HudWatchdogPrivate HudWatchdogPrivate;
struct _HudWatchdogPrivate {
	guint timeout;
	gulong timer;
	GMainLoop * loop;
};

#define HUD_WATCHDOG_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), HUD_WATCHDOG_TYPE, HudWatchdogPrivate))

static void hud_watchdog_class_init (HudWatchdogClass *klass);
static void hud_watchdog_init       (HudWatchdog *self);
static void hud_watchdog_dispose    (GObject *object);
static void hud_watchdog_finalize   (GObject *object);
static gboolean fire_watchdog       (gpointer user_data);

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
	self->priv = HUD_WATCHDOG_GET_PRIVATE(self);

	const gchar * envvar = g_getenv("HUD_SERVICE_TIMEOUT");
	if (envvar == NULL) {
		self->priv->timeout = DEFAULT_TIMEOUT;
	} else {
		self->priv->timeout = atoi(envvar);
	}

	self->priv->timer = g_timeout_add_seconds(self->priv->timeout, fire_watchdog, self);

	return;
}

static void
hud_watchdog_dispose (GObject *object)
{
	HudWatchdog * self = HUD_WATCHDOG(object);

	if (self->priv->timer != 0) {
		g_source_remove(self->priv->timer);
		self->priv->timer = 0;
	}

	G_OBJECT_CLASS (hud_watchdog_parent_class)->dispose (object);
	return;
}

static void
hud_watchdog_finalize (GObject *object)
{

	G_OBJECT_CLASS (hud_watchdog_parent_class)->finalize (object);
	return;
}

/* Oh noes!  It's our time to go!  Do this!  */
static gboolean
fire_watchdog (gpointer user_data)
{
	g_return_val_if_fail(IS_HUD_WATCHDOG(user_data), TRUE);
	HudWatchdog * self = HUD_WATCHDOG(user_data);

	g_debug("Firing Watchdog");

	if (self->priv->loop != NULL) {
		g_main_loop_quit(self->priv->loop);
	}

	return FALSE;
}
