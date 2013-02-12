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

#include "hudsourcelist.h"
#include "application-list.h"

/**
 * SECTION:hudsourcelist
 * @title: HudSourceList
 * @short_description: a list of #HudSources that is itself a #HudSource
 *
 * #HudSourceList is a list of #HudSources that functions as a
 * #HudSource itself.
 *
 * Calls to hud_source_search() on the list get turned into searches on
 * each source in the list.  If any source in the list emits the
 * HudSource::changed signal then the list itself will emit it as well.
 *
 * Sources may be added to the list using hud_source_list_add().  It is
 * not possible to remove sources.
 **/

/**
 * HudSourceList:
 *
 * This is an opaque structure type.
 **/

struct _HudSourceList
{
  GObject parent_instance;

  GSList *list;
};

typedef GObjectClass HudSourceListClass;

static void hud_source_list_iface_init (HudSourceInterface *iface);
G_DEFINE_TYPE_WITH_CODE (HudSourceList, hud_source_list, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (HUD_TYPE_SOURCE, hud_source_list_iface_init))

static void
hud_source_list_source_changed (HudSource *source,
                                gpointer   user_data)
{
  HudSourceList *list = user_data;

  hud_source_changed (HUD_SOURCE (list));
}

static void
hud_source_list_use (HudSource *source)
{
  HudSourceList *list = HUD_SOURCE_LIST (source);
  GSList *node;

  for (node = list->list; node; node = node->next)
    hud_source_use (node->data);
}

static void
hud_source_list_unuse (HudSource *source)
{
  HudSourceList *list = HUD_SOURCE_LIST (source);
  GSList *node;

  for (node = list->list; node; node = node->next)
    hud_source_unuse (node->data);
}

static void
hud_source_list_search (HudSource    *source,
                        HudTokenList *search_string,
                        void        (*append_func) (HudResult * result, gpointer user_data),
                        gpointer      user_data)
{
  HudSourceList *list = HUD_SOURCE_LIST (source);
  GSList *node;

  for (node = list->list; node; node = node->next)
    hud_source_search (node->data, search_string, append_func, user_data);
}

static void
hud_source_list_list_applications (HudSource    *source,
                                   HudTokenList *search_string,
                                   void        (*append_func) (const gchar *application_id, const gchar *application_icon, gpointer user_data),
                                   gpointer      user_data)
{
  HudSourceList *list = HUD_SOURCE_LIST (source);
  GSList *node;

  for (node = list->list; node; node = node->next)
    hud_source_list_applications (node->data, search_string, append_func, user_data);
}

static HudSource *
hud_source_list_get (HudSource   *source,
                     const gchar *application_id)
{
  HudSourceList *list = HUD_SOURCE_LIST (source);
  GSList *node;

  for (node = list->list; node; node = node->next) {
    HudSource *result = hud_source_get (node->data, application_id);
    if (result != NULL)
      return result;
  }

  return NULL;
}

static void
hud_source_list_activate_toolbar (HudSource *source, HudClientQueryToolbarItems item, GVariant *platform_data)
{
  HudSourceList *list = HUD_SOURCE_LIST (source);
  GSList *node;

  for (node = list->list; node; node = node->next)
    hud_source_activate_toolbar (node->data, item, platform_data);
}

/**
 * hud_source_list_get_items:
 * @list: a #HudSourceList
 *
 * Find the item collector that is associated with the active window.
 *
 * Return Value: (element-type HudItem) (transfer full) A list of #HudItem
 * objects.  Free with g_list_free_full(g_object_unref)
 */
static GList *
hud_source_list_get_items (HudSource *source)
{
  g_return_val_if_fail(HUD_IS_SOURCE_LIST(source), NULL);

  HudSourceList *list = HUD_SOURCE_LIST(source);
  GList *results = NULL;

  GSList *node;
  for (node = list->list; node; node = node->next) {
    if (HUD_IS_SOURCE(node->data))
    {
      HudSource * source = HUD_SOURCE(node->data);
      results = g_list_concat (results, hud_source_get_items (source));
    }
  }

  return results;
}

static void
hud_source_list_finalize (GObject *object)
{
  HudSourceList *list = HUD_SOURCE_LIST (object);

  /* signals have already been disconnected in dispose */
  g_slist_free_full (list->list, g_object_unref);

  G_OBJECT_CLASS (hud_source_list_parent_class)
    ->finalize (object);
}

static void
hud_source_list_init (HudSourceList *list)
{
}

static void
hud_source_list_iface_init (HudSourceInterface *iface)
{
  iface->use = hud_source_list_use;
  iface->unuse = hud_source_list_unuse;
  iface->search = hud_source_list_search;
  iface->list_applications = hud_source_list_list_applications;
  iface->get = hud_source_list_get;
  iface->get_items = hud_source_list_get_items;
  iface->activate_toolbar = hud_source_list_activate_toolbar;
}

static void
hud_source_list_class_init (HudSourceListClass *class)
{
  class->finalize = hud_source_list_finalize;
}

/**
 * hud_source_list_new:
 *
 * Creates a #HudSourceList.
 *
 * You should probably add some sources to it using
 * hud_source_list_add().
 *
 * Returns: a new empty #HudSourceList
 **/
HudSourceList *
hud_source_list_new (void)
{
  return g_object_new (HUD_TYPE_SOURCE_LIST, NULL);
}

/**
 * hud_source_list_add:
 * @list: a #HudSourceList
 * @source: a #HudSource to add to the list
 *
 * Adds @source to @list.
 *
 * Future hud_source_search() calls on @list will include results from
 * @source.
 **/
void
hud_source_list_add (HudSourceList *list,
                     HudSource     *source)
{
  g_return_if_fail (HUD_IS_SOURCE_LIST (list));
  g_return_if_fail (HUD_IS_SOURCE (source));

  g_signal_connect_object (source, "changed", G_CALLBACK (hud_source_list_source_changed), list, 0);
  list->list = g_slist_prepend (list->list, g_object_ref (source));

  hud_source_changed (HUD_SOURCE (source));
}

/**
 * hud_source_list_get_list:
 * @list: A #HudSourceList object
 *
 * Gets the list so you can look through it.
 *
 * Return value: (transfer none): The list of sources.
 */
GSList *
hud_source_list_get_list (HudSourceList * list)
{
	g_return_val_if_fail (HUD_IS_SOURCE_LIST (list), NULL);

	return list->list;
}
