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

#define G_LOG_DOMAIN "hudmenumodelcollector"

#include "hudmenumodelcollector.h"

#include "hudsource.h"
#include "hudresult.h"
#include "huditem.h"
#include "hudcollector.h"
#include "hudkeywordmapping.h"
#include "config.h"

#include <libbamf/libbamf.h>
#include <gio/gio.h>
#include <string.h>

/**
 * SECTION:hudmenumodelcollector
 * @title: HudMenuModelCollector
 * @short_description: a #HudSource that collects #HudItems from
 *   #GMenuModel
 *
 * The #HudMenuModelCollector collects menu items from the menus
 * associated with a window exported from an application using
 * #GMenuModel.  Activations are performed using #GActionGroup in the
 * usual way.
 *
 * The #GMenuModel is acquired using #GDBusMenuModel according to the
 * properties set on the #BamfWindow which must be passed to
 * hud_menu_model_collector_get().
 **/

/**
 * HudMenuModelCollector:
 *
 * This is an opaque structure type.
 **/

typedef struct _HudMenuModelContext HudMenuModelContext;

struct _HudMenuModelContext
{
  HudStringList *tokens;
  gchar *action_namespace;
  gint ref_count;
};

struct _HudMenuModelCollector
{
  HudCollector parent_instance;

  /* Cancelled on finalize */
  GCancellable *cancellable;

  /* GDBus shared session bus and D-Bus name of the app/indicator */
  GDBusConnection *session;
  gchar *unique_bus_name;

  /* GActionGroup's indexed by their prefix */
  GHashTable * action_groups;

  /* Boring details about the app/indicator we are showing. */
  gchar *app_id;
  gchar *icon;
  guint penalty;

  /* Class to map labels to keywords */
  HudKeywordMapping* keyword_mapping;

  /* Each time we see a new menumodel added we add it to 'models', start
   * watching it for changes and add its contents to 'items', possibly
   * finding more menumodels to do the same to.
   *
   * Each time an item is removed, we schedule an idle (in 'refresh_id')
   * to wipe out all the 'items', disconnect signals from each model in
   * 'models' and add them all back again.
   *
   * Searching just iterates over 'items'.
   */
  GPtrArray *items;
  GSList *models;
  guint refresh_id;

  /* Keep track of our use_count in order to send signals to HUD-aware
   * apps and indicators.
   */
  gint use_count;
};

typedef struct _model_data_t model_data_t;
struct _model_data_t {
	GMenuModel * model;
	gboolean is_hud_aware;
	GCancellable * cancellable;
	gchar * path;
	gchar * label;
};

typedef struct
{
  HudItem parent_instance;

  GRemoteActionGroup *group;
  gchar *action_name;
  GVariant *target;
} HudModelItem;

typedef HudItemClass HudModelItemClass;

/* Prototypes */
static void model_data_free                           (gpointer      data);
static void hud_menu_model_collector_hud_awareness_cb (GObject      *source,
                                                       GAsyncResult *result,
                                                       gpointer      user_data);
static GList * get_items (HudCollector * collector);

/* Functions */
static gchar *
hud_menu_model_context_get_prefix (HudMenuModelContext *context,
                                   const gchar         *action_name)
{
  if (context && context->action_namespace)
    /* Note: this will (intentionally) work if action_name is NULL */
    return g_strdup (context->action_namespace);
  else {
    gchar * retval = g_strdup (action_name);
    gchar * dot = g_strstr_len(retval, -1, ".");
    if (dot != NULL) {
      dot[0] = '\0';
    } else {
      retval[0] = '\0';
    }
    return retval;
  }
}

static HudStringList *
hud_menu_model_context_get_label (HudMenuModelContext *context,
                                  const gchar         *label)
{
  HudStringList *parent_tokens = context ? context->tokens : NULL;

  if (label)
    return hud_string_list_cons_label (label, parent_tokens);
  else
    return hud_string_list_ref (parent_tokens);
}

static HudStringList *
hud_menu_model_context_get_tokens (const gchar         *label,
                                  HudKeywordMapping   *keyword_mapping)
{
  HudStringList *tokens = NULL;

  if (label)
  {
    GPtrArray *keywords = hud_keyword_mapping_transform (keyword_mapping,
        label);
    gint i;
    for (i = 0; i < keywords->len; i++)
    {
      tokens = hud_string_list_cons_label (
          (gchar*) g_ptr_array_index(keywords, i), tokens);
    }
    return tokens;
  }
  else
    return NULL;
}

static HudMenuModelContext *
hud_menu_model_context_ref (HudMenuModelContext *context)
{
  if (context)
    g_atomic_int_inc (&context->ref_count);

  return context;
}

static void
hud_menu_model_context_unref (HudMenuModelContext *context)
{
  if (context && g_atomic_int_dec_and_test (&context->ref_count))
    {
      hud_string_list_unref (context->tokens);
      g_free (context->action_namespace);
      g_slice_free (HudMenuModelContext, context);
    }
}

static HudMenuModelContext *
hud_menu_model_context_new (HudMenuModelContext *parent,
                            const gchar         *namespace,
                            const gchar         *label,
                            HudKeywordMapping   *keyword_mapping)
{
  HudMenuModelContext *context;

  /* If we would be an unmodified copy of the parent, just take a ref */
  if (!namespace && !label)
    return hud_menu_model_context_ref (parent);

  context = g_slice_new (HudMenuModelContext);
  if (parent != NULL && parent->action_namespace != NULL) {
	  context->action_namespace = g_strjoin(".", parent->action_namespace, namespace, NULL);
  } else {
    context->action_namespace = g_strdup(namespace);
  }
  context->tokens = hud_menu_model_context_get_label (parent, label);
  context->ref_count = 1;

  return context;
}

G_DEFINE_TYPE (HudModelItem, hud_model_item, HUD_TYPE_ITEM)

static void
hud_model_item_activate (HudItem  *hud_item,
                         GVariant *platform_data)
{
  HudModelItem *item = (HudModelItem *) hud_item;

  g_remote_action_group_activate_action_full (item->group, item->action_name, item->target, platform_data);
}

static void
hud_model_item_finalize (GObject *object)
{
  HudModelItem *item = (HudModelItem *) object;

  g_object_unref (item->group);
  g_free (item->action_name);

  if (item->target)
    g_variant_unref (item->target);

  G_OBJECT_CLASS (hud_model_item_parent_class)
    ->finalize (object);
}

static void
hud_model_item_init (HudModelItem *item)
{
}

static void
hud_model_item_class_init (HudModelItemClass *class)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (class);

  gobject_class->finalize = hud_model_item_finalize;

  class->activate = hud_model_item_activate;
}

static HudItem *
hud_model_item_new (HudMenuModelCollector *collector,
                    HudMenuModelContext   *context,
                    const gchar           *label,
                    const gchar           *action_name,
                    const gchar           *accel,
                    GVariant              *target)
{
  HudModelItem *item;
  const gchar *stripped_action_name;
  gchar *prefix;
  GDBusActionGroup *group = NULL;
  HudStringList *full_label, *keywords;

  prefix = hud_menu_model_context_get_prefix (context, action_name);

  group = g_hash_table_lookup(collector->action_groups, prefix);

  if (!group)
    {
      g_free (prefix);
      return NULL;
    }

  /* Kinda silly, think we can do better here */
  if (g_str_has_prefix(action_name, prefix)) {
    stripped_action_name = action_name + strlen(prefix) + 1;
  } else {
    stripped_action_name = action_name;
  }

  full_label = hud_menu_model_context_get_label (context, label);
  keywords = hud_menu_model_context_get_tokens(label, collector->keyword_mapping);

  item = hud_item_construct (hud_model_item_get_type (), full_label, keywords, accel, collector->app_id, collector->icon, TRUE);
  item->group = g_object_ref (group);
  item->action_name = g_strdup (stripped_action_name);
  item->target = target ? g_variant_ref_sink (target) : NULL;

  hud_string_list_unref (full_label);
  hud_string_list_unref (keywords);
  g_free (prefix);

  return HUD_ITEM (item);
}

typedef HudCollectorClass HudMenuModelCollectorClass;

static void hud_menu_model_collector_iface_init (HudSourceInterface *iface);
G_DEFINE_TYPE_WITH_CODE (HudMenuModelCollector, hud_menu_model_collector, HUD_TYPE_COLLECTOR,
                         G_IMPLEMENT_INTERFACE (HUD_TYPE_SOURCE, hud_menu_model_collector_iface_init))

/* XXX: There is a potential for unbounded recursion here if a hostile
 * client were to feed us corrupted data.  We should figure out a way to
 * address that.
 *
 * It's not really a security problem for as long as we generally trust
 * the programs that we run (ie: under the same UID).  If we ever start
 * receiving menus from untrusted sources, we need to take another look,
 * though.
 */
static void hud_menu_model_collector_add_model_internal  (HudMenuModelCollector *collector,
                                                          GMenuModel            *model,
                                                          const gchar           *path,
                                                          HudMenuModelContext   *parent_context,
                                                          const gchar           *action_namespace,
                                                          const gchar           *label);

static void hud_menu_model_collector_disconnect (gpointer               data,
                                                 gpointer               user_data);

/* Takes a model data and adds it as a model */
static void
readd_models (gpointer data, gpointer user_data)
{
	HudMenuModelCollector *collector = user_data;
	model_data_t * model_data = data;

    hud_menu_model_collector_add_model_internal (collector, model_data->model, model_data->path, NULL, NULL, model_data->label);
	return;
}

static gboolean
hud_menu_model_collector_refresh (gpointer user_data)
{
  HudMenuModelCollector *collector = user_data;
  GSList *free_list;

  g_ptr_array_set_size (collector->items, 0);
  free_list = collector->models;
  collector->models = NULL;

  g_slist_foreach(free_list, readd_models, collector);

  g_slist_foreach (free_list, hud_menu_model_collector_disconnect, collector);
  g_slist_free_full (free_list, model_data_free);

  return G_SOURCE_REMOVE;
}

static GQuark
hud_menu_model_collector_context_quark ()
{
  static GQuark context_quark;

  if (!context_quark)
    context_quark = g_quark_from_string ("menu item context");

  return context_quark;
}

/* Function to convert from the GMenu format for the accel to something
   usable by humans. */
static gchar *
format_accel_for_users (gchar * accel)
{
	if (accel == NULL) {
		return g_strdup("");
	}

	GString * output = g_string_new("");
	gchar * head = accel;

	/* YEAH! String parsing, always my favorite. */
	while (head[0] != '\0') {
		if (head[0] == '<') {
			/* We're in modifier land */
			if (strncmp(head, "<Alt>", strlen("<Alt>")) == 0) {
				g_string_append(output, "Alt + ");
				head += strlen("<Alt>");
			} else if (strncmp(head, "<Primary>", strlen("<Primary>")) == 0) {
				g_string_append(output, "Ctrl + ");
				head += strlen("<Primary>");
			} else if (strncmp(head, "<Control>", strlen("<Control>")) == 0) {
				g_string_append(output, "Ctrl + ");
				head += strlen("<Control>");
			} else if (strncmp(head, "<Shift>", strlen("<Shift>")) == 0) {
				g_string_append(output, "Shift + ");
				head += strlen("<Shift>");
			} else if (strncmp(head, "<Super>", strlen("<Super>")) == 0) {
				g_string_append(output, "Super + ");
				head += strlen("<Super>");
			} else {
				/* Go to the close of the modifier */
				head = g_strstr_len(head, -1, ">") + 1;
			}
			continue;
		}

		g_string_append(output, head);
		break;
	}

	g_free(accel);

	return g_string_free(output, FALSE);
}

static void
hud_menu_model_collector_model_changed (GMenuModel *model,
                                        gint        position,
                                        gint        removed,
                                        gint        added,
                                        gpointer    user_data)
{
  HudMenuModelCollector *collector = user_data;
  HudMenuModelContext *context;
  gboolean changed;
  gint i;

  if (collector->refresh_id)
    /* We have a refresh scheduled already.  Ignore. */
    return;

  if (removed)
    {
      /* Items being removed is an unusual case.  Instead of having some
       * fancy algorithms for figuring out how to deal with that we just
       * start over again.
       *
       * ps: refresh_id is never set at this point (look up)
       */
      collector->refresh_id = g_idle_add (hud_menu_model_collector_refresh, collector);
      return;
    }

  context = g_object_get_qdata (G_OBJECT (model), hud_menu_model_collector_context_quark ());

  changed = FALSE;
  for (i = position; i < position + added; i++)
    {
      GMenuModel *link;
      gchar *label = NULL;
      gchar *action_namespace = NULL;
      gchar *action = NULL;
      gchar *accel = NULL;


      g_menu_model_get_item_attribute (model, i, "action-namespace", "s", &action_namespace);
      g_menu_model_get_item_attribute (model, i, G_MENU_ATTRIBUTE_ACTION, "s", &action);
      g_menu_model_get_item_attribute (model, i, G_MENU_ATTRIBUTE_LABEL, "s", &label);
      g_menu_model_get_item_attribute (model, i, "accel", "s", &accel);

      accel = format_accel_for_users(accel);

      /* Check if this is an action.  Here's where we may end up
       * creating a HudItem.
       */
      if (action && label)
        {
          GVariant *target;
          HudItem *item;

          target = g_menu_model_get_item_attribute_value (model, i, G_MENU_ATTRIBUTE_TARGET, NULL);

          item = hud_model_item_new (collector, context, label, action, accel, target);

          if (item)
            g_ptr_array_add (collector->items, item);

          if (target)
            g_variant_unref (target);

          changed = TRUE;
        }

      /* For 'section' and 'submenu' links, we should recurse.  This is
       * where the danger comes in (due to the possibility of unbounded
       * recursion).
       */
      if ((link = g_menu_model_get_item_link (model, i, G_MENU_LINK_SECTION)))
        {
          hud_menu_model_collector_add_model_internal (collector, link, NULL, context, action_namespace, label);
          g_object_unref (link);
        }

      if ((link = g_menu_model_get_item_link (model, i, G_MENU_LINK_SUBMENU)))
        {
          hud_menu_model_collector_add_model_internal (collector, link, NULL, context, action_namespace, label);
          g_object_unref (link);
        }

      g_free (action_namespace);
      g_free (action);
      g_free (label);
      g_free (accel);
    }

  if (changed)
    hud_source_changed (HUD_SOURCE (collector));
}

static void
hud_menu_model_collector_add_model_internal (HudMenuModelCollector *collector,
                                             GMenuModel            *model,
                                             const gchar           *path,
                                             HudMenuModelContext   *parent_context,
                                             const gchar           *action_namespace,
                                             const gchar           *label)
{
  g_return_if_fail(G_IS_MENU_MODEL(model));

  gint n_items;

  g_signal_connect (model, "items-changed", G_CALLBACK (hud_menu_model_collector_model_changed), collector);

  model_data_t * model_data = g_new0(model_data_t, 1);
  model_data->model = g_object_ref(model);
  model_data->is_hud_aware = FALSE;
  model_data->cancellable = g_cancellable_new();
  model_data->path = g_strdup(path);
  model_data->label = g_strdup(label);

  collector->models = g_slist_prepend (collector->models, model_data);

  if (path != NULL) {
    g_dbus_connection_call (collector->session, collector->unique_bus_name, path,
                            "com.canonical.hud.Awareness", "CheckAwareness",
                            NULL, G_VARIANT_TYPE_UNIT, G_DBUS_CALL_FLAGS_NONE, -1, model_data->cancellable,
                            hud_menu_model_collector_hud_awareness_cb, &model_data->is_hud_aware);
  }

  /* The tokens in 'context' are the list of strings that got us up to
   * where we are now, like "View > Toolbars".
   *
   * Strictly speaking, GMenuModel structures are DAGs, but we more or
   * less assume that they are trees here and replace the data
   * unconditionally when we visit it the second time (which will be
   * more or less never, because really, a menu is a tree).
   */
  g_object_set_qdata_full (G_OBJECT (model),
                           hud_menu_model_collector_context_quark (),
                           hud_menu_model_context_new (parent_context, action_namespace, label, collector->keyword_mapping),
                           (GDestroyNotify) hud_menu_model_context_unref);

  n_items = g_menu_model_get_n_items (model);
  if (n_items > 0)
    hud_menu_model_collector_model_changed (model, 0, 0, n_items, collector);
}

static void
hud_menu_model_collector_disconnect (gpointer data,
                                     gpointer user_data)
{
  g_signal_handlers_disconnect_by_func (data, hud_menu_model_collector_model_changed, user_data);
}

/* Sends an awareness to a model that needs it */
static void
set_awareness (HudMenuModelCollector * collector, model_data_t * model_data, gboolean active)
{
	if (!model_data->is_hud_aware) {
		return;
	}

	g_dbus_connection_call (collector->session, collector->unique_bus_name, model_data->path,
	                        "com.canonical.hud.Awareness", "HudActiveChanged", g_variant_new ("(b)", active),
	                        NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, NULL, NULL);

	return;
}

/* We're avoiding having to allocate a struct by using the function
   pointer here.  Kinda sneaky, but it works */
static void
make_aware (gpointer data, gpointer user_data)
{
	return set_awareness(HUD_MENU_MODEL_COLLECTOR(user_data), (model_data_t *)data, TRUE);
}

static void
make_unaware (gpointer data, gpointer user_data)
{
	return set_awareness(HUD_MENU_MODEL_COLLECTOR(user_data), (model_data_t *)data, FALSE);
}

/* Handles the activeness of this collector */
static void
hud_menu_model_collector_active_changed (HudMenuModelCollector *collector,
                                         gboolean               active)
{
  if (active) {
    g_slist_foreach(collector->models, make_aware, collector);
  } else {
    g_slist_foreach(collector->models, make_unaware, collector);
  }

  return;
}

static void
hud_menu_model_collector_use (HudSource *source)
{
  HudMenuModelCollector *collector = HUD_MENU_MODEL_COLLECTOR (source);

  if (collector->use_count == 0)
    hud_menu_model_collector_active_changed (collector, TRUE);

  collector->use_count++;
}

static void
hud_menu_model_collector_unuse (HudSource *source)
{
  HudMenuModelCollector *collector = HUD_MENU_MODEL_COLLECTOR (source);

  collector->use_count--;

  if (collector->use_count == 0)
    hud_menu_model_collector_active_changed (collector, FALSE);
}

static void
hud_menu_model_collector_search (HudSource    *source,
                                 HudTokenList *search_string,
                                 void        (*append_func) (HudResult * result, gpointer user_data),
                                 gpointer      user_data)
{
  HudMenuModelCollector *collector = HUD_MENU_MODEL_COLLECTOR (source);
  GPtrArray *items;
  gint i;

  items = collector->items;

  for (i = 0; i < items->len; i++)
    {
      HudResult *result;
      HudItem *item;

      item = g_ptr_array_index (items, i);
      result = hud_result_get_if_matched (item, search_string, collector->penalty);
      if (result)
        append_func(result, user_data);
    }
}

static void
hud_menu_model_collector_list_applications (HudSource    *source,
                                            HudTokenList *search_string,
                                            void        (*append_func) (const gchar *application_id, const gchar *application_icon, gpointer user_data),
                                            gpointer      user_data)
{
  HudMenuModelCollector *collector = HUD_MENU_MODEL_COLLECTOR (source);
  GPtrArray *items;
  gint i;

  items = collector->items;

  for (i = 0; i < items->len; i++)
    {
      HudResult *result;
      HudItem *item;

      item = g_ptr_array_index (items, i);
      result = hud_result_get_if_matched (item, search_string, collector->penalty);
      if (result) {
        append_func(collector->app_id, collector->icon, user_data);
        g_object_unref(result);
        break;
      }
    }
}

static HudSource *
hud_menu_model_collector_get (HudSource   *source,
                              const gchar *application_id)
{
  HudMenuModelCollector *collector = HUD_MENU_MODEL_COLLECTOR (source);

  if (g_strcmp0(collector->app_id, application_id) == 0)
    return source;

  return NULL;
}

/* Free's the model data structure */
static void
model_data_free (gpointer data)
{
	model_data_t * model_data = (model_data_t *)data;

	/* Make sure we don't have an operation outstanding */
	g_cancellable_cancel (model_data->cancellable);
	g_clear_object(&model_data->cancellable);

	g_clear_pointer(&model_data->path, g_free);
	g_clear_pointer(&model_data->label, g_free);

	g_clear_object(&model_data->model);
	g_free(model_data);

	return;
}

static void
hud_menu_model_collector_finalize (GObject *object)
{
  HudMenuModelCollector *collector = HUD_MENU_MODEL_COLLECTOR (object);

  g_cancellable_cancel (collector->cancellable);
  g_object_unref (collector->cancellable);

  if (collector->refresh_id)
    g_source_remove (collector->refresh_id);

  g_slist_free_full (collector->models, model_data_free);
  g_clear_pointer (&collector->action_groups, g_hash_table_unref);

  g_object_unref (collector->session);
  g_free (collector->unique_bus_name);
  g_free (collector->app_id);
  g_free (collector->icon);
  g_object_unref (collector->keyword_mapping);

  g_ptr_array_unref (collector->items);

  G_OBJECT_CLASS (hud_menu_model_collector_parent_class)
    ->finalize (object);
}

static void
hud_menu_model_collector_init (HudMenuModelCollector *collector)
{
  collector->items = g_ptr_array_new_with_free_func (g_object_unref);
  collector->cancellable = g_cancellable_new ();
  collector->action_groups = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_object_unref);
  collector->session = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, NULL);
}

static void
hud_menu_model_collector_iface_init (HudSourceInterface *iface)
{
  iface->use = hud_menu_model_collector_use;
  iface->unuse = hud_menu_model_collector_unuse;
  iface->search = hud_menu_model_collector_search;
  iface->list_applications = hud_menu_model_collector_list_applications;
  iface->get = hud_menu_model_collector_get;
}

static void
hud_menu_model_collector_class_init (HudMenuModelCollectorClass *class)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (class);

  gobject_class->finalize = hud_menu_model_collector_finalize;

  HudCollectorClass * cclass = HUD_COLLECTOR_CLASS(class);
  cclass->get_items = get_items;
}

static void
hud_menu_model_collector_hud_awareness_cb (GObject      *source,
                                           GAsyncResult *result,
                                           gpointer      user_data)
{
  GVariant *reply;

  /* The goal of this function is to set either the
   * app_menu_is_hud_aware or menubar_is_hud_aware flag (which we have a
   * pointer to in user_data) to TRUE in the case that the remote
   * appears to support the com.canonical.hud.Awareness protocol.
   *
   * If it supports it, the async call will be successful.  In that
   * case, we want to set *(gboolean *) user_data = TRUE;
   *
   * There are two cases that we don't want to do that write.  The first
   * is the event that the remote doesn't support the protocol.  In that
   * case, we will see an error when we inspect the result.  The other
   * is the case in which the flag to which user_data points no longer
   * exists (ie: collector has been finalized).  In this case, the
   * cancellable will have been cancelled and we will also see an error.
   *
   * Long story short: If we get any error, just do nothing.
   */

  reply = g_dbus_connection_call_finish (G_DBUS_CONNECTION (source), result, NULL);

  if (reply)
    {
      *(gboolean *) user_data = TRUE;
      g_variant_unref (reply);
    }
}

/**
 * hud_menu_model_collector_new:
 * @application_id: a unique identifier for the application
 * @icon: the icon for the appliction
 * @penalty: the penalty to apply to all results
 *
 * Create a new #HudMenuModelCollector object
 *
 * Return value: New #HudMenuModelCollector
 */
HudMenuModelCollector *
hud_menu_model_collector_new (const gchar *application_id,
                              const gchar *icon,
                              guint        penalty)
{
	HudMenuModelCollector * collector = g_object_new(HUD_TYPE_MENU_MODEL_COLLECTOR, NULL);

	collector->app_id = g_strdup (application_id);
	collector->icon = g_strdup (icon);
	collector->penalty = penalty;

	collector->keyword_mapping = hud_keyword_mapping_new();
	hud_keyword_mapping_load(collector->keyword_mapping, collector->app_id, DATADIR, GNOMELOCALEDIR);

	return collector;
}

/**
 * hud_menu_model_collector_add_window:
 * @window: a #BamfWindow
 *
 * If the given @window has #GMenuModel-style menus then returns a
 * collector for them, otherwise returns %NULL.
 *
 * Returns: a #HudMenuModelCollector, or %NULL
 **/
void
hud_menu_model_collector_add_window (HudMenuModelCollector * collector,
                                     BamfWindow  *window)
{
  g_return_if_fail(HUD_IS_MENU_MODEL_COLLECTOR(collector));

  gchar *unique_bus_name;
  gchar *application_object_path;
  gchar *window_object_path;
  gchar *app_menu_object_path;
  gchar *menubar_object_path;

  GDBusMenuModel * app_menu;
  GDBusMenuModel * menubar;

  unique_bus_name = bamf_window_get_utf8_prop (window, "_GTK_UNIQUE_BUS_NAME");

  if (!unique_bus_name)
    /* If this isn't set, we won't get very far... */
    return;

  if (!g_dbus_is_name(unique_bus_name)) {
    g_warning("Invalid BUS name '%s'", unique_bus_name);
    return;
  }

  if (collector->unique_bus_name == NULL) {
    collector->unique_bus_name = g_strdup(unique_bus_name);
  }

  if (g_strcmp0(unique_bus_name, collector->unique_bus_name) != 0) {
    g_warning("Bus name '%s' does not match '%s'", unique_bus_name, collector->unique_bus_name);
    g_free(unique_bus_name);
    return;
  }
  g_clear_pointer(&unique_bus_name, g_free);

  app_menu_object_path = bamf_window_get_utf8_prop (window, "_GTK_APP_MENU_OBJECT_PATH");
  menubar_object_path = bamf_window_get_utf8_prop (window, "_GTK_MENUBAR_OBJECT_PATH");
  application_object_path = bamf_window_get_utf8_prop (window, "_GTK_APPLICATION_OBJECT_PATH");
  window_object_path = bamf_window_get_utf8_prop (window, "_GTK_WINDOW_OBJECT_PATH");

  if (app_menu_object_path)
    {
      app_menu = g_dbus_menu_model_get (collector->session, collector->unique_bus_name, app_menu_object_path);
      hud_menu_model_collector_add_model_internal (collector, G_MENU_MODEL (app_menu), app_menu_object_path, NULL, NULL, NULL);
      g_object_unref(app_menu);
    }

  if (menubar_object_path)
    {
      menubar = g_dbus_menu_model_get (collector->session, collector->unique_bus_name, menubar_object_path);
      hud_menu_model_collector_add_model_internal (collector, G_MENU_MODEL (menubar), menubar_object_path, NULL, NULL, NULL);
      g_object_unref(menubar);
    }

  if (application_object_path)
    hud_menu_model_collector_add_actions(collector, G_ACTION_GROUP(g_dbus_action_group_get (collector->session, collector->unique_bus_name, application_object_path)), "app");

  if (window_object_path)
    hud_menu_model_collector_add_actions(collector, G_ACTION_GROUP(g_dbus_action_group_get (collector->session, collector->unique_bus_name, window_object_path)), "win");

  /* when the action groups change, we could end up having items
   * enabled/disabled.  how to deal with that?
   */

  g_free (application_object_path);
  g_free (window_object_path);

  return;
}

/**
 * hud_menu_model_collector_add_endpoint:
 * @prefix: the title to prefix to all items
 * @bus_name: a D-Bus bus name
 * @object_path: an object path at the destination given by @bus_name
 *
 * Creates a new #HudMenuModelCollector for the specified endpoint.
 *
 * This call is intended to be used for indicators.
 *
 * If @prefix is non-%NULL (which, for indicators, it ought to be), then
 * it is prefixed to every item created by the collector.
 *
 * If @penalty is non-zero then all results returned from the collector
 * have their distance increased by a percentage equal to the penalty.
 * This allows items from indicators to score lower than they would
 * otherwise.
 *
 * Returns: a new #HudMenuModelCollector
 */
void
hud_menu_model_collector_add_endpoint (HudMenuModelCollector * collector,
                                       const gchar *prefix,
                                       const gchar *bus_name,
                                       const gchar *object_path)
{
  g_return_if_fail(HUD_IS_MENU_MODEL_COLLECTOR(collector));

  hud_menu_model_collector_add_actions(collector, G_ACTION_GROUP(g_dbus_action_group_get (collector->session, bus_name, object_path)), NULL);

  GDBusMenuModel * app_menu = g_dbus_menu_model_get (collector->session, bus_name, object_path);
  hud_menu_model_collector_add_model(collector, G_MENU_MODEL (app_menu), prefix);
  g_object_unref(app_menu);

  return;
}

/**
 * hud_menu_model_collector_add_mode:
 * @collector: A #HudMenuModelCollector object
 * @model: Model to add
 * @prefix: (allow none): Text prefix to add to all entries if needed
 *
 * Adds a Menu Model to the collector.
 */
void
hud_menu_model_collector_add_model (HudMenuModelCollector * collector, GMenuModel * model, const gchar * prefix)
{
	g_return_if_fail(HUD_IS_MENU_MODEL_COLLECTOR(collector));
	g_return_if_fail(G_IS_MENU_MODEL(model));

	return hud_menu_model_collector_add_model_internal(collector, model, NULL, NULL, NULL, prefix);
}

/**
 * hud_menu_model_collector_add_actions:
 * @collector: A #HudMenuModelCollector object
 * @group: Action Group to add
 * @prefix: (allow none): Text prefix to add to all entries if needed
 *
 * Add a set of actios to the collector
 */
void
hud_menu_model_collector_add_actions (HudMenuModelCollector * collector, GActionGroup * group, const gchar * prefix)
{
	g_return_if_fail(HUD_IS_MENU_MODEL_COLLECTOR(collector));
	g_return_if_fail(G_IS_ACTION_GROUP(group));

	gchar * local_prefix = NULL;
	if (prefix == NULL) {
		local_prefix = g_strdup("");
	} else {
		local_prefix = g_strdup(prefix);
	}

	g_hash_table_insert(collector->action_groups, local_prefix, group);

	return;
}

/**
 * hud_menu_model_collector_get_items:
 * @collector: A #HudMenuModelCollector
 *
 * Get the list of items that are currently being watched
 *
 * Return value: (transfer full) (element-type HudItem): Items to look at
 */
static GList *
get_items (HudCollector * collector)
{
	g_return_val_if_fail(HUD_IS_MENU_MODEL_COLLECTOR(collector), NULL);
	HudMenuModelCollector * mcollector = HUD_MENU_MODEL_COLLECTOR(collector);

	GList * retval = NULL;
	int i;
	for (i = 0; i < mcollector->items->len; i++) {
		retval = g_list_prepend(retval, g_object_ref(g_ptr_array_index(mcollector->items, i)));
	}

	return retval;
}
