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

#ifndef __HUD_OPERATION_H__
#define __HUD_OPERATION_H__

#pragma GCC visibility push(default)

#include <gio/gio.h>

G_BEGIN_DECLS

#define HUD_TYPE_OPERATION                                  (hud_operation_get_type ())
#define HUD_OPERATION(inst)                                 (G_TYPE_CHECK_INSTANCE_CAST ((inst),                     \
                                                             HUD_TYPE_OPERATION, HudOperation))
#define HUD_OPERATION_CLASS(class)                          (G_TYPE_CHECK_CLASS_CAST ((class),                       \
                                                             HUD_TYPE_OPERATION, HudOperationClass))
#define HUD_IS_OPERATION(inst)                              (G_TYPE_CHECK_INSTANCE_TYPE ((inst),                     \
                                                             HUD_TYPE_OPERATION))
#define HUD_IS_OPERATION_CLASS(class)                       (G_TYPE_CHECK_CLASS_TYPE ((class),                       \
                                                             HUD_TYPE_OPERATION))
#define HUD_OPERATION_GET_CLASS(inst)                       (G_TYPE_INSTANCE_GET_CLASS ((inst),                      \
                                                             HUD_TYPE_OPERATION, HudOperationClass))

typedef struct _HudOperationPrivate                            HudOperationPrivate;
typedef struct _HudOperationClass                              HudOperationClass;
typedef struct _HudOperation                                   HudOperation;

struct _HudOperation
{
  GSimpleActionGroup   parent_instance;
  HudOperationPrivate *priv;
};

struct _HudOperationClass
{
  GSimpleActionGroupClass parent_class;

  void (* started)  (HudOperation *operation);

  void (* changed)  (HudOperation *operation,
                     const gchar  *parameter_name);

  void (* ended)    (HudOperation *operation);
};

GType                   hud_operation_get_type                          (void) G_GNUC_CONST;

void                    hud_operation_setup                             (HudOperation *operation,
                                                                         GVariant     *parameters);

gpointer                hud_operation_get_user_data                     (HudOperation *operation);

gboolean                hud_operation_get_boolean                       (HudOperation *operation,
                                                                         const gchar  *action_name);
gint                    hud_operation_get_int                           (HudOperation *operation,
                                                                         const gchar  *action_name);
guint                   hud_operation_get_uint                          (HudOperation *operation,
                                                                         const gchar  *action_name);
gdouble                 hud_operation_get_double                        (HudOperation *operation,
                                                                         const gchar  *action_name);

G_END_DECLS

#pragma GCC visibility push(default)

#endif /* __HUD_OPERATION_H__ */
