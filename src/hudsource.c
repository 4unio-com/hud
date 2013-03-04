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

#define G_LOG_DOMAIN "hudsource"

#include "hudsource.h"

/**
 * SECTION:hudsource
 * @title: HudSource
 * @short_description: a source of #HudResults
 *
 * A #HudSource is a very simple interface with only two APIs.
 *
 * First, a #HudSource may be searched, with a string.  The search
 * results in a list of #HudResults being returned.  This is
 * hud_source_search().
 *
 * Second, a #HudSource has a simple "changed" signal that is emitted
 * whenever the result of calling hud_source_search() may have changed.
 *
 * A #HudSource is stateless with respect to active queries.
 *
 * FUTURE DIRECTIONS:
 *
 * #HudSource should probably have an API to indicate if there is an
 * active query in progress.  There are two reasons for this.  First is
 * because the implementations could be more lazy with respect to
 * watching for changes.  Second is because dbusmenu has a concept of if
 * a menu is being shown or not and some applications may find this
 * information to be useful.
 *
 * It may also make sense to handle queries in a more stateful way,
 * possibly replacing the "change" signal with something capable of
 * expressing more fine-grained changes (eg: a single item was added or
 * removed).
 **/

/**
 * HudSource:
 *
 * This is an opaque structure type.
 **/

/**
 * HudSourceInterface:
 * @g_iface: the #GTypeInterface
 * @search: virtual function pointer for hud_source_search()
 *
 * This is the interface vtable for #HudSource.
 **/

G_DEFINE_INTERFACE (HudSource, hud_source, G_TYPE_OBJECT)

static gulong hud_source_changed_signal;

static void
hud_source_default_init (HudSourceInterface *iface)
{
  /**
   * HudSource::changed:
   * @source: a #HudSource
   *
   * Indicates that the #HudSource may have changed.  After this signal,
   * calls to hud_source_search() may return different results than they
   * did before.
   **/
  hud_source_changed_signal = g_signal_new ("changed", HUD_TYPE_SOURCE, G_SIGNAL_RUN_LAST, 0, NULL,
                                            NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}

/**
 * hud_source_use:
 * @source; a #HudSource
 *
 * Mark a #HudSource as "in use" (ie: actively being queried).
 *
 * The source maintains a use count.  You must call hud_source_unuse()
 * for as many times as you called hud_source_use().
 *
 * The source may not emit change signals unless it is marked as being
 * used (although it is free to ignore this hint and emit them anyway).
 * Some data in the source may also be out of date.  It is therefore
 * recommended that calls to hud_source_search() be preceeded by a call
 * to this function.
 **/
void
hud_source_use (HudSource *source)
{
  g_return_if_fail (HUD_IS_SOURCE (source));

  g_debug ("use on %s %p", G_OBJECT_TYPE_NAME (source), source);

  HudSourceInterface * iface = HUD_SOURCE_GET_IFACE (source);
  if (iface->use != NULL) {
    return iface->use(source);
  }
}

/**
 * hud_source_unuse:
 * @source; a #HudSource
 *
 * Reverses the effect of a previous call to hud_source_use().
 *
 * The source maintains a use count.  You must call hud_source_unuse()
 * for as many times as you called hud_source_use().
 **/
void
hud_source_unuse (HudSource *source)
{
  g_return_if_fail (HUD_IS_SOURCE (source));

  g_debug ("unuse on %s %p", G_OBJECT_TYPE_NAME (source), source);

  HudSourceInterface * iface = HUD_SOURCE_GET_IFACE (source);
  if (iface->unuse != NULL) {
    return iface->unuse(source);
  }
}

/**
 * hud_source_search:
 * @source: a #HudSource
 * @results_array: (element-type HudResult): array to append results to
 * @search_string: the search string
 *
 * Searches for #HudItems in @source that potentially match
 * @search_string and creates #HudResults for them, appending them to
 * @results_array.
 *
 * @source will emit a ::changed signal if the results of calling this
 * function may have changed, at which point you should call it again.
 **/
void
hud_source_search (HudSource    *source,
                   HudTokenList *search_string,
                   void        (*append_func) (HudResult * result, gpointer user_data),
                   gpointer      user_data)
{
  g_debug ("search on %s %p", G_OBJECT_TYPE_NAME (source), source);

  HudSourceInterface * iface = HUD_SOURCE_GET_IFACE (source);
  if (iface->unuse != NULL) {
    return iface->search(source, search_string, append_func, user_data);
  }
}

void
hud_source_list_applications (HudSource    *source,
                              HudTokenList *search_tokens,
                              void        (*append_func) (const gchar *application_id, const gchar *application_icon, HudSourceItemType type, gpointer user_data),
                              gpointer      user_data)
{
  g_debug ("list_applications on %s %p", G_OBJECT_TYPE_NAME (source), source);

  HudSourceInterface * iface = HUD_SOURCE_GET_IFACE (source);
  if (iface->list_applications != NULL) {
    return iface->list_applications(source, search_tokens, append_func, user_data);
  }
}

/**
 * hud_source_get:
 * @source; a #HudSource
 *
 * Mark a #HudSource as "in use" (ie: actively being queried).
 *
 * Gets the last source that is responsible of giving results for application_id
 **/
HudSource *
hud_source_get (HudSource   *source,
                const gchar *application_id)
{
  g_return_val_if_fail (HUD_IS_SOURCE (source), NULL);

  g_debug ("get on %s %p", G_OBJECT_TYPE_NAME (source), source);

  HudSourceInterface * iface = HUD_SOURCE_GET_IFACE (source);
  if (iface->get != NULL) {
    return iface->get(source, application_id);
  }

  return NULL;
}

/**
 * hud_source_activate_toolbar:
 * @source; a #HudSource
 *
 * Activate a toolbar item on a source
 **/
void
hud_source_activate_toolbar (HudSource * source, HudClientQueryToolbarItems item, GVariant *platform_data)
{
  g_return_if_fail (HUD_IS_SOURCE (source));

  g_debug ("activate toolbar on %s %p", G_OBJECT_TYPE_NAME (source), source);

  HudSourceInterface * iface = HUD_SOURCE_GET_IFACE (source);
  if (iface->activate_toolbar != NULL) {
    return iface->activate_toolbar(source, item, platform_data);
  }

  return;
}

/**
 * hud_source_get_items:
 * @collector: a #HudDbusmenuCollector
 *
 * Gets the items that have been collected at any point in time.
 *
 * Return Value: (element-type HudItem) (transfer full) A list of #HudItem
 * objects.  Free with g_list_free_full(g_object_unref)
 */
GList *
hud_source_get_items (HudSource *source)
{
  g_return_val_if_fail(HUD_IS_SOURCE(source), NULL);

  g_debug ("get_items on %s %p", G_OBJECT_TYPE_NAME (source), source);

  HudSourceInterface * iface = HUD_SOURCE_GET_IFACE (source);
  if (iface->get_items != NULL) {
    return iface->get_items(source);
  }

  return NULL;
}

/**
 * hud_source_changed:
 * @source: a #HudSource
 *
 * Signals that @source may have changed (ie: emits the "changed"
 * signal).
 *
 * This function should only ever be called by implementations of
 * #HudSource.
 **/
void
hud_source_changed (HudSource *source)
{
  g_signal_emit (source, hud_source_changed_signal, 0);
}

/**
 * hud_source_get_app_id:
 * @source: a #HudSource
 *
 * Get the application ID.  Shouldn't be implemented by list
 * sources.
 *
 * Return value: The ID of the application
 */
const gchar *
hud_source_get_app_id (HudSource * source)
{
  g_return_val_if_fail(HUD_IS_SOURCE(source), NULL);

  HudSourceInterface * iface = HUD_SOURCE_GET_IFACE (source);
  if (iface->get_app_id != NULL) {
    return iface->get_app_id(source);
  }

  return NULL;
}

/**
 * hud_source_get_app_icon:
 * @source: a #HudSource
 *
 * Get the application icon.  Shouldn't be implemented by list
 * sources.
 *
 * Return value: The icon of the application
 */
const gchar *
hud_source_get_app_icon (HudSource * source)
{
  g_return_val_if_fail(HUD_IS_SOURCE(source), NULL);

  HudSourceInterface * iface = HUD_SOURCE_GET_IFACE (source);
  if (iface->get_app_icon != NULL) {
    return iface->get_app_icon(source);
  }

  return NULL;
}
