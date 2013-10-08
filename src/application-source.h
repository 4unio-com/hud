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

#ifndef __HUD_APPLICATION_SOURCE_H__
#define __HUD_APPLICATION_SOURCE_H__

#include <glib-object.h>
#include "window-info.h"
#include "application-source-context.h"

G_BEGIN_DECLS

#define HUD_TYPE_APPLICATION_SOURCE            (hud_application_source_get_type ())
#define HUD_APPLICATION_SOURCE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), HUD_TYPE_APPLICATION_SOURCE, HudApplicationSource))
#define HUD_APPLICATION_SOURCE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), HUD_TYPE_APPLICATION_SOURCE, HudApplicationSourceClass))
#define HUD_IS_APPLICATION_SOURCE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HUD_TYPE_APPLICATION_SOURCE))
#define HUD_IS_APPLICATION_SOURCE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), HUD_TYPE_APPLICATION_SOURCE))
#define HUD_APPLICATION_SOURCE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), HUD_TYPE_APPLICATION_SOURCE, HudApplicationSourceClass))

typedef struct _HudApplicationSource         HudApplicationSource;
typedef struct _HudApplicationSourceClass    HudApplicationSourceClass;
typedef struct _HudApplicationSourcePrivate  HudApplicationSourcePrivate;

typedef struct _HudCollector                 HudCollector;

struct _HudApplicationSourceClass {
	GObjectClass parent_class;
};

struct _HudApplicationSource {
	GObject parent;
	HudApplicationSourcePrivate * priv;
};

GType                    hud_application_source_get_type          (void);
HudApplicationSource *   hud_application_source_new_for_app       (HudApplicationInfo *     bapp);
HudApplicationSource *   hud_application_source_new_for_id        (const gchar *            id);
gboolean                 hud_application_source_is_empty          (HudApplicationSource *   app);
void                     hud_application_source_focus             (HudApplicationSource *   app,
                                                                   HudApplicationInfo *     bapp,
                                                                   HudWindowInfo *          window);
const gchar *            hud_application_source_get_path          (HudApplicationSource *   app);
const gchar *            hud_application_source_get_id            (HudApplicationSource *   app);
const gchar *            hud_application_source_get_app_icon      (HudApplicationSource *   app);
HudApplicationInfo *     hud_application_source_get_application_info (HudApplicationSource *   app);
void                     hud_application_source_add_window        (HudApplicationSource *   app,
                                                                   HudWindowInfo *          window);
void                     hud_application_source_window_closed     (HudApplicationSource *   app,
                                                                   guint                    xid);
gboolean                 hud_application_source_has_xid           (HudApplicationSource *   app,
                                                                   guint32                  xid);

void                     hud_application_source_set_context       (HudApplicationSource *   app,
                                                                   guint32                  xid,
                                                                   const gchar *            context);
const gchar *            hud_application_source_get_context       (HudApplicationSource *   app,
                                                                   guint32                  xid);
void                     hud_application_source_add_context       (HudApplicationSource *   app,
                                                                   HudApplicationSourceContext * context);
void                     hud_application_source_set_focused_win   (HudApplicationSource *   app,
                                                                   guint32                  xid);

G_END_DECLS

#endif
