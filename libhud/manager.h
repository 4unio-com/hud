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

#ifndef __HUD_MANAGER_H__
#define __HUD_MANAGER_H__

#include <glib.h>

void                    hud_manager_say_hello                           (const gchar *application_id,
                                                                         const gchar *description_path);

void                    hud_manager_say_goodbye                         (const gchar *application_id);

void                    hud_manager_add_actions                         (const gchar *application_id,
                                                                         const gchar *prefix,
                                                                         GVariant    *identifier,
                                                                         const gchar *object_path);

void                    hud_manager_remove_actions                      (const gchar *application_id,
                                                                         const gchar *prefix,
                                                                         GVariant    *identifier);

#endif /* __HUD_MANAGER_H__ */
