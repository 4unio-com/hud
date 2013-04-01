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

#ifndef __HUD_GTK_MANAGER_H__
#define __HUD_GTK_MANAGER_H__

#include <gtk/gtk.h>
#include <hud.h>

#pragma GCC visibility push(default)
G_BEGIN_DECLS

#define HUD_GTK_TYPE_MANAGER            (hud_gtk_manager_get_type ())
#define HUD_GTK_MANAGER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), HUD_GTK_TYPE_MANAGER, HudGtkManager))
#define HUD_GTK_MANAGER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), HUD_GTK_TYPE_MANAGER, HudGtkManagerClass))
#define HUD_GTK_IS_MANAGER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HUD_GTK_TYPE_MANAGER))
#define HUD_GTK_IS_MANAGER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), HUD_GTK_TYPE_MANAGER))
#define HUD_GTK_MANAGER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), HUD_GTK_TYPE_MANAGER, HudGtkManagerClass))

typedef struct _HudGtkManager         HudGtkManager;
typedef struct _HudGtkManagerClass    HudGtkManagerClass;
typedef struct _HudGtkManagerPrivate  HudGtkManagerPrivate;

struct _HudGtkManagerClass {
	HudManagerClass parent_class;
};

struct _HudGtkManager {
	HudManager parent;
	HudGtkManagerPrivate * priv;
};

GType hud_gtk_manager_get_type (void);

HudGtkManager *         hud_gtk_manager_new               (GtkApplication *  app);
HudActionPublisher *    hud_gtk_manager_get_publisher     (HudGtkManager *   manager,
                                                           GVariant *        id);

G_END_DECLS
#pragma GCC visibility pop

#endif /* __HUD_GTK_MANAGER_H__ */
