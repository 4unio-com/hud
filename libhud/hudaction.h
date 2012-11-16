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

#ifndef __HUD_ACTION_H__
#define __HUD_ACTION_H__

#include "hudoperation.h"

G_BEGIN_DECLS

#define HUD_TYPE_ACTION                                     (hud_action_get_type ())
#define HUD_ACTION(inst)                                    (G_TYPE_CHECK_INSTANCE_CAST ((inst),                     \
                                                             HUD_TYPE_ACTION, HudAction))
#define HUD_ACTION_CLASS(class)                             (G_TYPE_CHECK_CLASS_CAST ((class),                       \
                                                             HUD_TYPE_ACTION, HudActionClass))
#define HUD_IS_ACTION(inst)                                 (G_TYPE_CHECK_INSTANCE_TYPE ((inst),                     \
                                                             HUD_TYPE_ACTION))
#define HUD_IS_ACTION_CLASS(class)                          (G_TYPE_CHECK_CLASS_TYPE ((class),                       \
                                                             HUD_TYPE_ACTION))
#define HUD_ACTION_GET_CLASS(inst)                          (G_TYPE_INSTANCE_GET_CLASS ((inst),                      \
                                                             HUD_TYPE_ACTION, HudActionClass))

typedef struct _HudActionPrivate                            HudActionPrivate;
typedef struct _HudActionClass                              HudActionClass;
typedef struct _HudAction                                   HudAction;

struct _HudAction
{
  GObject           parent_instance;
  HudActionPrivate *priv;
};

struct _HudActionClass
{
  GObjectClass parent_class;

  void (* create_operation) (HudAction *action,
                             GVariant  *parameters);
};

GType                   hud_action_get_type                             (void) G_GNUC_CONST;

HudAction *             hud_action_new                                  (const gchar  *name);

void                    hud_action_set_enabled                          (HudAction    *action,
                                                                         gboolean      enabled);

gboolean                hud_action_get_is_active                        (HudAction    *action);

HudOperation *          hud_action_get_operation                        (HudAction    *action);

void                    hud_action_set_operation                        (HudAction    *action,
                                                                         HudOperation *operation);

G_END_DECLS

#endif /* __HUD_ACTION_H__ */
