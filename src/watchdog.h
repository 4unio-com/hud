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

#ifndef __HUD_WATCHDOG_H__
#define __HUD_WATCHDOG_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define HUD_WATCHDOG_TYPE            (hud_watchdog_get_type ())
#define HUD_WATCHDOG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), HUD_WATCHDOG_TYPE, HudWatchdog))
#define HUD_WATCHDOG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), HUD_WATCHDOG_TYPE, HudWatchdogClass))
#define IS_HUD_WATCHDOG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HUD_WATCHDOG_TYPE))
#define IS_HUD_WATCHDOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), HUD_WATCHDOG_TYPE))
#define HUD_WATCHDOG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), HUD_WATCHDOG_TYPE, HudWatchdogClass))

typedef struct _HudWatchdog        HudWatchdog;
typedef struct _HudWatchdogClass   HudWatchdogClass;
typedef struct _HudWatchdogPrivate HudWatchdogPrivate;

struct _HudWatchdogClass {
	GObjectClass parent_class;
};

struct _HudWatchdog {
	GObject parent;
	HudWatchdogPrivate * priv;
};

GType hud_watchdog_get_type (void);

HudWatchdog * hud_watchdog_new  (GMainLoop * loop);
void          hud_watchdog_ping (HudWatchdog * watchdog);
guint         hud_watchdog_get_timeout (HudWatchdog * watchdog);

G_END_DECLS

#endif
