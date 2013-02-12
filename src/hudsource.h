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

#ifndef __HUD_SOURCE_H__
#define __HUD_SOURCE_H__

#include "huditem.h"
#include "hudresult.h"
#include "enum-types.h"

#define HUD_TYPE_SOURCE                                     (hud_source_get_type ())
#define HUD_SOURCE(inst)                                    (G_TYPE_CHECK_INSTANCE_CAST ((inst),                     \
                                                             HUD_TYPE_SOURCE, HudSource))
#define HUD_IS_SOURCE(inst)                                 (G_TYPE_CHECK_INSTANCE_TYPE ((inst),                     \
                                                             HUD_TYPE_SOURCE))
#define HUD_SOURCE_GET_IFACE(inst)                          (G_TYPE_INSTANCE_GET_INTERFACE ((inst),                  \
                                                             HUD_TYPE_SOURCE, HudSourceInterface))

typedef struct _HudSourceInterface                          HudSourceInterface;
typedef struct _HudSource                                   HudSource;

struct _HudSourceInterface
{
  GTypeInterface g_iface;

  void (* use)    (HudSource    *source);
  void (* unuse)  (HudSource    *source);
  void (* search) (HudSource    *source,
                   HudTokenList *search_tokens,
                   void        (*append_func) (HudResult * result, gpointer user_data),
                   gpointer      user_data);
  void (* list_applications) (HudSource    *source,
                              HudTokenList *search_tokens,
                              void        (*append_func) (const gchar *application_id, const gchar *application_icon, gpointer user_data),
                              gpointer      user_data);
  HudSource * (* get) (HudSource    *source,
                       const gchar *application_id);

  GList * (* get_items) (HudSource    *source);
  void (* activate_toolbar) (HudSource *                source,
                             HudClientQueryToolbarItems item,
                             GVariant                  *platform_data);
};

GType                   hud_source_get_type                             (void);

void                    hud_source_use                                  (HudSource    *source);
void                    hud_source_unuse                                (HudSource    *source);

void                    hud_source_search                               (HudSource    *source,
                                                                         HudTokenList *search_tokens,
                                                                         void        (*append_func) (HudResult * result, gpointer user_data),
                                                                         gpointer      user_data);

void                    hud_source_list_applications                    (HudSource    *source,
                                                                         HudTokenList *search_tokens,
                                                                         void        (*append_func) (const gchar *application_id, const gchar *application_icon, gpointer user_data),
                                                                         gpointer      user_data);

HudSource *             hud_source_get                                  (HudSource    *source,
                                                                         const gchar  *application_id);

GList *                 hud_source_get_items                            (HudSource    *source);

void                    hud_source_changed                              (HudSource    *source);

void                    hud_source_activate_toolbar                     (HudSource *   source,
                                                                         HudClientQueryToolbarItems item,
                                                                         GVariant     *platform_data);

#endif /* __HUD_SOURCE_H__ */
