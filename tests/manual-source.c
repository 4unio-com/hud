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

#include "hudmanualsource.h"

#include "source.h"
#include "result.h"

struct _HudManualSource
{
  GObject parent_instance;
  gchar *application_id;
  gchar *app_icon;

  GPtrArray *items;
  gint use_count;
};

typedef GObjectClass HudManualSourceClass;

static void hud_manual_source_iface_init (HudSourceInterface *iface);
G_DEFINE_TYPE_WITH_CODE (HudManualSource, hud_manual_source, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (HUD_TYPE_SOURCE, hud_manual_source_iface_init))

void
hud_manual_source_add (HudManualSource *self, HudStringList *tokens, HudStringList *keywords,
    const gchar *shortcut, gboolean enabled)
{
  HudItem *item = hud_item_new (tokens, keywords, shortcut, self->application_id, self->app_icon, NULL, enabled);
  g_ptr_array_add(self->items, item);
  hud_source_changed (HUD_SOURCE (self));
}

static void
hud_manual_source_use (HudSource *hud_source)
{
  HudManualSource *source = HUD_MANUAL_SOURCE (hud_source);
  source->use_count++;
}

static void
hud_manual_source_unuse (HudSource *hud_source)
{
  HudManualSource *source = HUD_MANUAL_SOURCE (hud_source);
  source->use_count--;
}

typedef struct
{
  HudTokenList *search_string;
  void (*append_func) (HudResult * result, gpointer user_data);
  gpointer list;
} HudManualSourceForEach;

static void
hud_manual_source_foreach (gpointer item, gpointer user_data)
{
  HudManualSourceForEach *data = (HudManualSourceForEach*) user_data;

  HudResult *result = hud_result_get_if_matched (item, data->search_string, 0);
  if (result != NULL )
  {
    data->append_func (result, data->list);
  }
}

static void
hud_manual_source_search (HudSource    *hud_source,
                         HudTokenList *search_string,
                         void        (*append_func) (HudResult * result, gpointer user_data),
                         gpointer      list)
{
  HudManualSource *self = HUD_MANUAL_SOURCE (hud_source);
  HudManualSourceForEach user_data = {search_string, append_func, list};
  g_ptr_array_foreach(self->items, hud_manual_source_foreach, &user_data);
}

static void
hud_manual_source_list_applications (HudSource    *hud_source,
                                     HudTokenList *search_string,
                                     void        (*append_func) (const gchar *application_id, const gchar *application_icon, HudSourceItemType type, gpointer user_data),
                                     gpointer      user_data)
{
  HudManualSource *self = HUD_MANUAL_SOURCE (hud_source);
  guint i = 0;
  for ( ; i < self->items->len; ++i) {
      HudItem *item = HUD_ITEM(g_ptr_array_index(self->items, i));
      HudResult *result = hud_result_get_if_matched (item, search_string, 0);
      if (result != NULL )
      {
        append_func (self->application_id, self->app_icon, HUD_SOURCE_ITEM_TYPE_BACKGROUND_APP, user_data);
        g_object_unref(result);
        break;
      }
  }
}

static HudSource *
hud_manual_source_get (HudSource    *hud_source,
                       const gchar *application_id)
{
  HudManualSource *self = HUD_MANUAL_SOURCE (hud_source);
  if (g_strcmp0(self->application_id, application_id) == 0)
    return hud_source;
  return NULL;
}

static const gchar *
hud_manual_source_get_app_id (HudSource * hud_source)
{
  HudManualSource *self = HUD_MANUAL_SOURCE (hud_source);
  return self->application_id;
}

static const gchar *
hud_manual_source_get_app_icon (HudSource * hud_source)
{
  HudManualSource *self = HUD_MANUAL_SOURCE (hud_source);
  return self->app_icon;
}

static void
hud_manual_source_copy_item (gpointer data, gpointer user_data)
{
  GList **result = (GList **) user_data;
  *result = g_list_append(*result, g_object_ref(data));
}

static GList *
hud_manual_source_get_items (HudSource *source)
{
  HudManualSource *self = HUD_MANUAL_SOURCE (source);
  GList *result = NULL;
  g_ptr_array_foreach(self->items, hud_manual_source_copy_item, &result);
  return result;
}

static void
hud_manual_source_finalize (GObject *object)
{
  HudManualSource *self = HUD_MANUAL_SOURCE (object);

  g_ptr_array_free(self->items, TRUE);
  g_free(self->application_id);
  g_free(self->app_icon);

  G_OBJECT_CLASS (hud_manual_source_parent_class)
    ->finalize (object);
}

static void
hud_manual_source_init (HudManualSource *self)
{
  self->use_count = 0;
  self->items = g_ptr_array_new_with_free_func(g_object_unref);
}

static void
hud_manual_source_iface_init (HudSourceInterface *iface)
{
  iface->use = hud_manual_source_use;
  iface->unuse = hud_manual_source_unuse;
  iface->search = hud_manual_source_search;
  iface->list_applications = hud_manual_source_list_applications;
  iface->get = hud_manual_source_get;
  iface->get_app_id = hud_manual_source_get_app_id;
  iface->get_app_icon = hud_manual_source_get_app_icon;
  iface->get_items = hud_manual_source_get_items;
}

static void
hud_manual_source_class_init (HudManualSourceClass *class)
{
  class->finalize = hud_manual_source_finalize;
}

/**
 * hud_manual_source_new:
 *
 * Creates a #HudManualSource.
 *
 * Returns: a new empty #HudManualSource
 **/
HudManualSource *
hud_manual_source_new (const gchar *application_id, const gchar *app_icon)
{
  HudManualSource *source = g_object_new (HUD_TYPE_MANUAL_SOURCE, NULL);
  source->application_id = g_strdup(application_id);
  source->app_icon = g_strdup(app_icon);
  return source;
}
