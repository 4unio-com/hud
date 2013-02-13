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

#include "huddebugsource.h"

#include "hudsource.h"
#include "hudresult.h"

/**
 * SECTION:huddebugsource
 * @title: HudDebugSource
 * @short_description: a source to assist with debugging
 *
 * #HudDebugSource is a source for debugging purposes.  It is not
 * enabled by default but you can have it added to the global list of
 * sources by setting the HUD_DEBUG_SOURCE environment variable.
 *
 * Presently it creates a #HudItem corresponding to the current date and
 * time (and updates it as the time changes).
 **/

/**
 * HudDebugSource:
 *
 * This is an opaque structure type.
 **/

struct _HudDebugSource
{
  GObject parent_instance;

  HudItem *item;
  gint use_count;
  gint timeout;
  HudSourceItemType type;
};

typedef GObjectClass HudDebugSourceClass;

static void hud_debug_source_iface_init (HudSourceInterface *iface);
G_DEFINE_TYPE_WITH_CODE (HudDebugSource, hud_debug_source, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (HUD_TYPE_SOURCE, hud_debug_source_iface_init))

static gboolean
hud_debug_source_timeout (gpointer user_data)
{
  HudDebugSource *source = user_data;
  HudStringList *tokens;
  GDateTime *now;
  gchar *time;

  g_clear_object (&source->item);

  now = g_date_time_new_now_local ();
  time = g_date_time_format (now, "hud-debug time: %c");
  tokens = hud_string_list_cons (time, NULL);
  g_date_time_unref (now);
  g_free (time);

  source->item = hud_item_new (tokens, NULL, NULL, NULL, NULL, TRUE);
  hud_string_list_unref (tokens);

  hud_source_changed (HUD_SOURCE (source));

  return TRUE;
}

static void
hud_debug_source_use (HudSource *hud_source)
{
  HudDebugSource *source = HUD_DEBUG_SOURCE (hud_source);

  if (source->use_count == 0)
    source->timeout = g_timeout_add (1000, hud_debug_source_timeout, source);

  source->use_count++;
}

static void
hud_debug_source_unuse (HudSource *hud_source)
{
  HudDebugSource *source = HUD_DEBUG_SOURCE (hud_source);

  source->use_count--;

  if (source->use_count == 0)
    {
      g_source_remove (source->timeout);
      source->timeout = 0;
    }
}

static void
hud_debug_source_search (HudSource    *hud_source,
                         HudTokenList *search_string,
                         void        (*append_func) (HudResult * result, gpointer user_data),
                         gpointer      user_data)
{
  HudDebugSource *source = HUD_DEBUG_SOURCE (hud_source);

  if (source->item)
    {
      HudResult *result;

      result = hud_result_get_if_matched (source->item, search_string, 0);
      if (result != NULL)
        append_func(result, user_data);
    }
}

static void
hud_debug_source_list_applications (HudSource    *hud_source,
                                    HudTokenList *search_string,
                                    void        (*append_func) (const gchar *application_id, const gchar *application_icon, HudSourceItemType type, gpointer user_data),
                                    gpointer      user_data)
{
  HudDebugSource *source = HUD_DEBUG_SOURCE (hud_source);

  if (source->item)
    {
      HudResult *result;

      result = hud_result_get_if_matched (source->item, search_string, 0);
      if (result != NULL) {
        append_func(hud_item_get_app_id(source->item), hud_item_get_app_icon(source->item), source->type, user_data);
        g_object_unref (result);
      }
    }
}

static HudSource *
hud_debug_source_get (HudSource     *hud_source,
                      const gchar   *application_id)
{
  HudDebugSource *source = HUD_DEBUG_SOURCE (hud_source);

  if (source->item)
    {
      if (g_strcmp0 (application_id, hud_item_get_app_id(source->item)) == 0)
        return hud_source;
    }

  return NULL;
}

static void
hud_debug_source_finalize (GObject *object)
{
  HudDebugSource *source = HUD_DEBUG_SOURCE (object);

  g_clear_object (&source->item);

  if (source->timeout)
    g_source_remove (source->timeout);

  G_OBJECT_CLASS (hud_debug_source_parent_class)
    ->finalize (object);
}

static void
hud_debug_source_init (HudDebugSource *source)
{
  source->type = HUD_SOURCE_ITEM_TYPE_BACKGROUND_APP;
}

static void
hud_debug_source_iface_init (HudSourceInterface *iface)
{
  iface->use = hud_debug_source_use;
  iface->unuse = hud_debug_source_unuse;
  iface->search = hud_debug_source_search;
  iface->list_applications = hud_debug_source_list_applications;
  iface->get = hud_debug_source_get;
}

static void
hud_debug_source_class_init (HudDebugSourceClass *class)
{
  class->finalize = hud_debug_source_finalize;
}

/**
 * hud_debug_source_new:
 *
 * Creates a #HudDebugSource.
 *
 * Returns: a new empty #HudDebugSource
 **/
HudDebugSource *
hud_debug_source_new (void)
{
  return g_object_new (HUD_TYPE_DEBUG_SOURCE, NULL);
}
