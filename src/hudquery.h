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

#ifndef __HUD_QUERY_H__
#define __HUD_QUERY_H__

#include "hudsource.h"
#include "hudresult.h"
#include "application-list.h"
#include "watchdog.h"

#define HUD_TYPE_QUERY                                      (hud_query_get_type ())
#define HUD_QUERY(inst)                                     (G_TYPE_CHECK_INSTANCE_CAST ((inst),                     \
                                                             HUD_TYPE_QUERY, HudQuery))
#define HUD_IS_QUERY(inst)                                  (G_TYPE_CHECK_INSTANCE_TYPE ((inst),                     \
                                                             HUD_TYPE_QUERY))
typedef struct _DeeModel DeeModel;
typedef struct _GDBusConnection               GDBusConnection;

typedef struct _HudQuery                                    HudQuery;

GType                   hud_query_get_type                              (void);

HudQuery *              hud_query_new                                   (HudSource   *all_sources,
                                                                         HudWatchdog  * watchdog,
                                                                         HudApplicationList  * application_list,
                                                                         const gchar *search_string,
                                                                         gint         num_results,
                                                                         GDBusConnection *connection,
                                                                         const gchar * sender,
                                                                         const guint  query_count);

void                    hud_query_close                                 (HudQuery    *query);
void                    hud_query_update_search                         (HudQuery    *  query,
                                                                         const gchar *  search_string);

const gchar *           hud_query_get_path                              (HudQuery    *query);
const gchar *           hud_query_get_results_name                      (HudQuery    *query);
const gchar *           hud_query_get_appstack_name                     (HudQuery    *query);
const gchar *           hud_query_get_query                             (HudQuery    *query);
guint                   hud_query_get_number                            (HudQuery    *query);
DeeModel    *           hud_query_get_results_model                     (HudQuery    *query);
DeeModel    *           hud_query_get_appstack_model                    (HudQuery    *query);

#endif /* __HUD_QUERY_H__ */
