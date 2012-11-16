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

#ifndef __HUD_OPERATION_H__
#define __HUD_OPERATION_H__

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
  GObject           parent_instance;
  HudOperationPrivate *priv;
};

struct _HudOperationClass
{
  GObjectClass parent_class;

  void (* start)    (HudOperation *operation);

  void (* update)   (HudOperation *operation,
                     const gchar  *parameter_name);

  void (* response) (HudOperation *operation,
                     const gchar  *response_id,
                     GVariant     *response_data);

  void (* end)      (HudOperation *operation);
};

GType                   hud_operation_get_type                          (void) G_GNUC_CONST;

G_END_DECLS

#endif /* __HUD_OPERATION_H__ */
