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

#ifndef __HUD_COLLECTOR_H__
#define __HUD_COLLECTOR_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define HUD_TYPE_COLLECTOR            (hud_collector_get_type ())
#define HUD_COLLECTOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), HUD_TYPE_COLLECTOR, HudCollector))
#define HUD_COLLECTOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), HUD_TYPE_COLLECTOR, HudCollectorClass))
#define HUD_IS_COLLECTOR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HUD_TYPE_COLLECTOR))
#define HUD_IS_COLLECTOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), HUD_TYPE_COLLECTOR))
#define HUD_COLLECTOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), HUD_TYPE_COLLECTOR, HudCollectorClass))

typedef struct _HudCollector      HudCollector;
typedef struct _HudCollectorClass HudCollectorClass;

struct _HudCollectorClass {
	GObjectClass parent_class;

	GList * (*get_items) (HudCollector * collector);
};

struct _HudCollector {
	GObject parent;
};

GType    hud_collector_get_type     (void);
GList *  hud_collector_get_items    (HudCollector * collector);

G_END_DECLS

#endif
