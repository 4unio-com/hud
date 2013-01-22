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

#ifndef __HUD_MANAGER_H__
#define __HUD_MANAGER_H__

#include <glib-object.h>

#include "action-publisher.h"

G_BEGIN_DECLS

#define HUD_TYPE_MANAGER            (hud_manager_get_type ())
#define HUD_MANAGER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), HUD_TYPE_MANAGER, HudManager))
#define HUD_MANAGER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), HUD_TYPE_MANAGER, HudManagerClass))
#define HUD_IS_MANAGER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HUD_TYPE_MANAGER))
#define HUD_IS_MANAGER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), HUD_TYPE_MANAGER))
#define HUD_MANAGER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), HUD_TYPE_MANAGER, HudManagerClass))

#define HUD_MANAGER_PROP_APPLICATION  "application"
#define HUD_MANAGER_PROP_APP_ID       "app-id"

typedef struct _HudManager        HudManager;
typedef struct _HudManagerClass   HudManagerClass;
typedef struct _HudManagerPrivate HudManagerPrivate;

struct _HudManagerClass {
	GObjectClass parent_class;
};

struct _HudManager {
	GObject parent;
	HudManagerPrivate * priv;
};

GType                   hud_manager_get_type                            (void);

HudManager *            hud_manager_new                                 (const gchar * application_id);

HudManager *            hud_manager_new_for_application                 (GApplication * application);

void                    hud_manager_add_actions                         (HudManager * manager,
                                                                         HudActionPublisher * pub);

void                    hud_manager_remove_actions                      (HudManager * manager,
                                                                         HudActionPublisher * pub);

G_END_DECLS

#endif /* __HUD_MANAGER_H__ */
