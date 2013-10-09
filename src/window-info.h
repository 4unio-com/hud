/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Pete Woods <pete.woods@canonical.com>
 */


#ifndef __HUD_WINDOW_INFO_H__
#define __HUD_WINDOW_INFO_H__

#include <glib-object.h>
#include <window-stack-iface.h>

/*
 * Type macros.
 */
#define HUD_TYPE_WINDOW_INFO                  (hud_window_info_get_type ())
#define HUD_WINDOW_INFO(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), HUD_TYPE_WINDOW_INFO, HudWindowInfo))
#define HUD_IS_WINDOW_INFO(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HUD_TYPE_WINDOW_INFO))
#define HUD_WINDOW_INFO_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), HUD_TYPE_WINDOW_INFO, HudWindowInfoClass))
#define HUD_IS_WINDOW_INFO_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), HUD_TYPE_WINDOW_INFO))
#define HUD_WINDOW_INFO_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), HUD_TYPE_WINDOW_INFO, HudWindowInfoClass))

typedef struct _HudWindowInfo        HudWindowInfo;

#define WINDOW_ID_ALL_WINDOWS (0)

typedef enum {
  HUD_WINDOW_INFO_STAGE_MAIN,
  HUD_WINDOW_INFO_STAGE_SIDE,
  HUD_WINDOW_INFO_STAGE_WINDOWED,
} HudWindowInfoStage;

typedef HudWindowInfo HudApplicationInfo;

GType hud_window_info_get_type (void);

/*
 * Method definitions.
 */

HudWindowInfo * hud_window_info_new(DBusWindowStack *window_stack,
		guint window_id, const gchar *app_id, guint stack);

guint32 hud_window_info_get_window_id(HudWindowInfo *self);

const gchar * hud_window_info_get_app_id(HudWindowInfo *self);

const gchar * hud_window_info_get_desktop_file(HudWindowInfo *self);

gchar * hud_window_info_get_utf8_prop(HudWindowInfo *self,
		const gchar *property_name);

gchar ** hud_window_info_get_utf8_properties(HudWindowInfo *self,
		const gchar * const *property_names);

#endif /* __HUD_WINDOW_INFO_H__ */
