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
 * Author: Ryan Lortie <desrt@desrt.ca>
 */

#ifndef __HUD_MENU_MODEL_COLLECTOR_H__
#define __HUD_MENU_MODEL_COLLECTOR_H__

#include <gio/gio.h>

#include "abstract-app.h"

#define HUD_TYPE_MENU_MODEL_COLLECTOR                       (hud_menu_model_collector_get_type ())
#define HUD_MENU_MODEL_COLLECTOR(inst)                      (G_TYPE_CHECK_INSTANCE_CAST ((inst),                     \
                                                             HUD_TYPE_MENU_MODEL_COLLECTOR,                          \
                                                             HudMenuModelCollector))
#define HUD_IS_MENU_MODEL_COLLECTOR(inst)                   (G_TYPE_CHECK_INSTANCE_TYPE ((inst),                     \
                                                             HUD_TYPE_MENU_MODEL_COLLECTOR))

typedef struct _HudMenuModelCollector                       HudMenuModelCollector;

GType                   hud_menu_model_collector_get_type               (void);

HudMenuModelCollector * hud_menu_model_collector_new                    (const gchar *application_id,
                                                                         const gchar *icon,
                                                                         guint        penalty);

void                    hud_menu_model_collector_add_window             (HudMenuModelCollector * collector,
                                                                         AbstractWindow *        window);

void                    hud_menu_model_collector_add_endpoint           (HudMenuModelCollector * collector,
                                                                         const gchar *prefix,
                                                                         const gchar *bus_name,
                                                                         const gchar *menu_path,
                                                                         const gchar *action_path);

void                    hud_menu_model_collector_add_model              (HudMenuModelCollector * collector,
                                                                         GMenuModel *   model,
                                                                         const gchar *  prefix);

void                    hud_menu_model_collector_add_actions            (HudMenuModelCollector * collector,
                                                                         GActionGroup *   group,
                                                                         const gchar *    prefix);

#endif /* __HUD_MENU_MODEL_COLLECTOR_H__ */
