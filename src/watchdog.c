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

	if (self->priv->timeout != 0) {
		self->priv->timer = g_timeout_add_seconds(self->priv->timeout, fire_watchdog, self);
	}

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
//	HudWatchdog * self = HUD_WATCHDOG(user_data);

//	g_debug("Firing Watchdog");

//	if (self->priv->loop != NULL) {
//		g_main_loop_quit(self->priv->loop);
//	}

	return FALSE;
}

/**
 * hud_watchdog_new:
 * @loop: Mainloop to quit if we timeout
 *
 * Sets up a watchdog that will quit on the main loop if it
 * doesn't get enough attention.  Reminds of an girlfriend
 * I had once...
 *
 * Return value: (transfer full): A new #HudWatchdog object
 */
HudWatchdog *
hud_watchdog_new (GMainLoop * loop)
{
	HudWatchdog * watchdog = g_object_new(HUD_WATCHDOG_TYPE, NULL);

	watchdog->priv->loop = loop;

	return watchdog;
}

/**
 * hud_watchdog_ping:
 * @watchdog: Watchdog to give attention to
 *
 * Makes sure to startover and not timeout.
 */
void
hud_watchdog_ping (HudWatchdog * watchdog)
{
	/* Doing a silent fail on NULL so that our tests can not worry about
	   setting up dummy watchdogs when just testing the query.  They have
	   their own timeouts */
	if (watchdog == NULL) {
		return;
	}

	g_return_if_fail(IS_HUD_WATCHDOG(watchdog));

	if (watchdog->priv->timer != 0) {
		g_source_remove(watchdog->priv->timer);
		watchdog->priv->timer = 0;
	}

	if (watchdog->priv->timeout != 0) {
		watchdog->priv->timer = g_timeout_add_seconds(watchdog->priv->timeout, fire_watchdog, watchdog);
	}
	return;
}

/**
 * hud_watchdog_get_timeout:
 * @watchdog: Watchdog to interegate
 *
 * Get the timeout of this watchdog.
 *
 * Return value: The number of seconds before it goes off
 */
guint
hud_watchdog_get_timeout (HudWatchdog * watchdog)
{
	g_return_val_if_fail(IS_HUD_WATCHDOG(watchdog), 0);

	return watchdog->priv->timeout;
}
