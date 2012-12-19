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
 * Author: Ryan Lortie <desrt@desrt.ca>
 */

#if !defined (_HUD_H_INSIDE) && !defined (HUD_COMPILATION)
#error "Only <hud.h> can be included directly."
#endif

#ifndef __HUD_ACTION_PUBLISHER_H__
#define __HUD_ACTION_PUBLISHER_H__

#include <gio/gio.h>

G_BEGIN_DECLS

#define HUD_TYPE_ACTION_PUBLISHER                           (hud_action_publisher_get_type ())
#define HUD_ACTION_PUBLISHER(inst)                          (G_TYPE_CHECK_INSTANCE_CAST ((inst),                     \
                                                             HUD_TYPE_ACTION_PUBLISHER, HudActionPublisher))
#define HUD_IS_ACTION_PUBLISHER(inst)                       (G_TYPE_CHECK_INSTANCE_TYPE ((inst),                     \
                                                             HUD_TYPE_ACTION_PUBLISHER))

#define HUD_TYPE_ACTION_DESCRIPTION                         (hud_action_description_get_type ())
#define HUD_ACTION_DESCRIPTION(inst)                        (G_TYPE_CHECK_INSTANCE_CAST ((inst),                     \
                                                             HUD_TYPE_ACTION_DESCRIPTION, HudActionDescription))
#define HUD_IS_ACTION_DESCRIPTION(inst)                     (G_TYPE_CHECK_INSTANCE_TYPE ((inst),                     \
                                                             HUD_TYPE_ACTION_DESCRIPTION))

typedef struct _HudActionDescription                        HudActionDescription;
typedef struct _HudActionPublisher                          HudActionPublisher;

GType                   hud_action_publisher_get_type                   (void) G_GNUC_CONST;

HudActionPublisher *    hud_action_publisher_new_for_id                 (GVariant              *id);

void                    hud_action_publisher_add_action_description     (HudActionPublisher    *publisher,
                                                                         HudActionDescription  *description);

void                    hud_action_publisher_add_descriptions_from_file (HudActionPublisher    *publisher,
                                                                         const gchar           *filename);

void                    hud_action_publisher_add_action_group           (HudActionPublisher    *publisher,
                                                                         const gchar           *prefix,
                                                                         GVariant              *identifier,
                                                                         const gchar           *object_path);
void                    hud_action_publisher_remove_action_group        (HudActionPublisher    *publisher,
                                                                         const gchar           *prefix,
                                                                         GVariant              *identifier);

HudActionDescription *  hud_action_description_new                      (const gchar           *action_name,
                                                                         GVariant              *action_target);
HudActionDescription *  hud_action_description_ref                      (HudActionDescription  *description);
void                    hud_action_description_unref                    (HudActionDescription  *description);
const gchar *           hud_action_description_get_action_name          (HudActionDescription  *description);
GVariant *              hud_action_description_get_action_target        (HudActionDescription  *description);
void                    hud_action_description_set_attribute_value      (HudActionDescription  *description,
                                                                         const gchar           *attribute_name,
                                                                         GVariant              *value);
void                    hud_action_description_set_attribute            (HudActionDescription  *description,
                                                                         const gchar           *attribute_name,
                                                                         const gchar           *format_string,
                                                                         ...);

G_END_DECLS

#endif /* __HUD_ACTION_PUBLISHER_H__ */
