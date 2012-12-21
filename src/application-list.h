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

#ifndef __HUD_APPLICATION_LIST_H__
#define __HUD_APPLICATION_LIST_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define HUD_TYPE_APPLICATION_LIST            (hud_application_list_get_type ())
#define HUD_APPLICATION_LIST(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), HUD_TYPE_APPLICATION_LIST, HudApplicationList))
#define HUD_APPLICATION_LIST_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), HUD_TYPE_APPLICATION_LIST, HudApplicationListClass))
#define HUD_IS_APPLICATION_LIST(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HUD_TYPE_APPLICATION_LIST))
#define HUD_IS_APPLICATION_LIST_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), HUD_TYPE_APPLICATION_LIST))
#define HUD_APPLICATION_LIST_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), HUD_TYPE_APPLICATION_LIST, HudApplicationListClass))

typedef struct _HudApplicationList          HudApplicationList;
typedef struct _HudApplicationListClass     HudApplicationListClass;
typedef struct _HudApplicationListPrivate   HudApplicationListPrivate;

struct _HudApplicationListClass {
	GObjectClass parent_class;
};

struct _HudApplicationList {
	GObject parent;
	HudApplicationListPrivate * priv;
};

GType               hud_application_list_get_type       (void);

G_END_DECLS

#endif /* __HUD_APPLICATION_LIST_H__ */

