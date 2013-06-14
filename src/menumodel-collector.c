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

#include "menumodel-collector.h"

#include "source.h"
#include "result.h"
#include "item.h"
#include "keyword-mapping.h"
#include "action-muxer.h"

#include <gio/gio.h>
#include <string.h>

#define RECURSE_DATA        "hud-menu-model-recurse-level"
#define EXPORT_PATH         "hud-menu-model-export-path"
#define EXPORT_MENU         "hud-menu-model-export-menu"

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
  GObject parent_instance;

  /* Cancelled on finalize */
  GCancellable *cancellable;

  /* GDBus shared session bus and D-Bus name of the app/indicator */
  GDBusConnection *session;
  gchar *unique_bus_name;

  /* GActionGroup's indexed by their prefix */
  GActionMuxer * muxer;
  GArray * agroups;

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

  /* This is the export path that we've been given */
  gchar * base_export_path;
  guint muxer_export;

  HudSourceItemType type;

  /* Track what we have to not add it twice */
  GHashTable * base_models;
};

/* Structure for when we're tracking a model, all the info
   that we need to keep on it. */
typedef struct _model_data_t model_data_t;
struct _model_data_t {
	GDBusConnection *session;

	GMenuModel * model;
	gboolean is_hud_aware;
	GCancellable * cancellable;
	gchar * path;
	gchar * label;
	guint recurse;
};

/* Signals on the action group to know when it changes */
typedef struct _action_group_signals_t action_group_signals_t;
struct _action_group_signals_t {
	GActionGroup * group;
	gulong action_added;
	gulong action_enabled_changed;
	gulong action_removed;
	gulong action_state_changed;
};

/* Structure to pass two values in a single pointer, amazing! */
typedef struct _exported_menu_id_t exported_menu_id_t;
struct _exported_menu_id_t {
	guint id;
	GDBusConnection * bus;
	GMenu * export;
	gchar * path;
};

struct _HudModelItem {
  HudItem parent_instance;

  GRemoteActionGroup *group;
  gchar *action_name;
  gchar *action_name_full;
  gchar *action_path;
  GVariant *target;
  HudClientQueryToolbarItems toolbar_item;

  GMenuModel * submodel;
};

typedef HudItemClass HudModelItemClass;

/* Prototypes */
static void model_data_free                           (gpointer      data);
static void hud_menu_model_collector_hud_awareness_cb (GObject      *source,
                                                       GAsyncResult *result,
                                                       gpointer      user_data);
static GList * hud_menu_model_collector_get_items (HudSource * source);
static void hud_menu_model_collector_get_toolbar_entries (HudSource * source,
                                                          GArray * toolbar);
static void hud_menu_model_collector_activate_toolbar (HudSource *   source,
                                                       HudClientQueryToolbarItems titem,
                                                       GVariant  *   platform_data);
static const gchar * hud_menu_model_collector_get_app_icon (HudSource * collector);
static const gchar * hud_menu_model_collector_get_app_id (HudSource * collector);

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
hud_model_item_activate_toolbar (HudModelItem  *hud_item,
                                 HudClientQueryToolbarItems item,
                                 GVariant *platform_data)
{
	if (hud_item->toolbar_item == item) {
		hud_item_activate(HUD_ITEM(hud_item), platform_data);
	}

	return;
}

static void
hud_model_item_finalize (GObject *object)
{
  HudModelItem *item = (HudModelItem *) object;

  g_clear_object(&item->submodel);
  g_object_unref (item->group);
  g_free (item->action_name);
  g_free (item->action_name_full);
  g_free (item->action_path);

  if (item->target)
    g_variant_unref (item->target);

  G_OBJECT_CLASS (hud_model_item_parent_class)
    ->finalize (object);
}

static void
hud_model_item_init (HudModelItem *item)
{
  item->toolbar_item = -1;
}

static void
hud_model_item_class_init (HudModelItemClass *class)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (class);

  gobject_class->finalize = hud_model_item_finalize;

  class->activate = hud_model_item_activate;
}

/* Breaks apart the string and adds it to the list */
static HudStringList *
append_keywords_string (HudStringList * list, const gchar * string)
{
	if (string == NULL) {
		return list;
	}

	/* Limiting to 64 keywords.  A bit arbitrary, but we need to ensure
	   that bad apps can't hurt us. */
	gchar ** split = g_strsplit(string, ";", 64);

	if (split == NULL) {
		return list;
	}

	int i;
	for (i = 0; split[i] != NULL; i++) {
		if (split[i][0] != '\0')
			list = hud_string_list_cons(split[i], list);
	}

	g_strfreev(split);
	return list;
}

static HudItem *
hud_model_item_new (HudMenuModelCollector *collector,
                    HudMenuModelContext   *context,
                    const gchar           *label,
                    const gchar           *action_name,
                    const gchar           *accel,
                    const gchar           *description,
                    const gchar           *keywords_string,
                    GVariant              *target,
                    const gchar           *toolbar)
{
  HudModelItem *item;
  const gchar *stripped_action_name;
  gchar *prefix;
  GActionGroup *group = NULL;
  HudStringList *full_label, *keywords;

  prefix = hud_menu_model_context_get_prefix (context, action_name);

  if (prefix[0] == '\0')
    g_clear_pointer(&prefix, g_free);

  group = g_action_muxer_get(collector->muxer, prefix);

  if (!group)
    {
      g_free (prefix);
      return NULL;
    }

  /* Kinda silly, think we can do better here */
  if (prefix != NULL && g_str_has_prefix(action_name, prefix)) {
    stripped_action_name = action_name + strlen(prefix) + 1;
  } else {
    stripped_action_name = action_name;
  }

  full_label = hud_menu_model_context_get_label (context, label);
  keywords = hud_menu_model_context_get_tokens(label, collector->keyword_mapping);
  keywords = append_keywords_string(keywords, keywords_string);

  item = hud_item_construct (hud_model_item_get_type (), full_label, keywords, accel, collector->app_id, collector->icon, description, TRUE);
  item->group = g_object_ref (group);
  item->action_name = g_strdup (stripped_action_name);
  item->action_name_full = g_strdup (action_name);
  item->target = target ? g_variant_ref_sink (target) : NULL;

  if (toolbar != NULL)
    {
      item->toolbar_item = hud_client_query_toolbar_items_get_value_from_nick(toolbar);
    }

  hud_string_list_unref (full_label);
  hud_string_list_unref (keywords);
  g_free (prefix);

  return HUD_ITEM (item);
}

/* Set the submenu property for the item */
static void
hud_model_item_set_submenu (HudModelItem * item, GMenuModel * submenu, const gchar * action_path)
{
	g_return_if_fail(G_IS_MENU_MODEL(submenu));
	g_return_if_fail(g_object_get_data(G_OBJECT(submenu), EXPORT_PATH) != NULL);

	item->submodel = g_object_ref(submenu);
	item->action_path = g_strdup(action_path);

	return;
}

/**
 * hud_model_item_is_toolbar:
 * @item: A #HudModelItem to check
 *
 * Check to see if an item has a toolbar representation
 *
 * Return value: Whether this is in the toolbar
 */
gboolean
hud_model_item_is_toolbar (HudModelItem * item)
{
	g_return_val_if_fail(HUD_IS_MODEL_ITEM(item), FALSE);

	return item->toolbar_item != -1;
}

/**
 * hud_model_item_is_parameterized:
 * @item: A #HudModelItem to check
 *
 * Check to see if an item represents a parameterized set of
 * actions.
 *
 * Return value: Whether this has parameterized actions or not
 */
gboolean
hud_model_item_is_parameterized (HudModelItem * item)
{
	g_return_val_if_fail(HUD_IS_MODEL_ITEM(item), FALSE);

	return item->submodel != NULL;
}

/**
 * hud_model_item_activate_parameterized:
 * @item: A #HudModelItem to check
 * @timestamp: The time of the user event from the dispaly server
 * @base_action: Action to activate the base events on
 * @action_path: Path that the actions can be found on
 * @model_path: Path to the models
 * @section: Model section to use
 * 
 * Uses the item to find all the information about creating a parameterized
 * view of the item in the HUD.
 */
void
hud_model_item_activate_parameterized (HudModelItem * item, guint32 timestamp, const gchar ** base_action, const gchar ** action_path, const gchar ** model_path, gint * section)
{
	g_return_if_fail(HUD_IS_MODEL_ITEM(item));

	/* Make sure we have a place to put things, we require it */
	g_return_if_fail(base_action != NULL);
	g_return_if_fail(action_path != NULL);
	g_return_if_fail(model_path != NULL);
	g_return_if_fail(section != NULL);
	g_return_if_fail(hud_model_item_is_parameterized(item));

	*base_action = item->action_name_full;
	*model_path = (const gchar *)g_object_get_data(G_OBJECT(item->submodel), EXPORT_PATH);
	*action_path = item->action_path;
	*section = 1;

	hud_item_mark_usage(HUD_ITEM(item));

	return;
}

typedef GObjectClass HudMenuModelCollectorClass;

static void hud_menu_model_collector_iface_init (HudSourceInterface *iface);
G_DEFINE_TYPE_WITH_CODE (HudMenuModelCollector, hud_menu_model_collector, G_TYPE_OBJECT,
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
                                                          const gchar           *label,
                                                          guint                  recurse,
                                                          HudSourceItemType      type);

static void hud_menu_model_collector_disconnect (gpointer               data,
                                                 gpointer               user_data);

/* Takes a model data and adds it as a model */
static void
readd_models (gpointer data, gpointer user_data)
{
	HudMenuModelCollector *collector = user_data;
	model_data_t * model_data = data;

    hud_menu_model_collector_add_model_internal (collector, model_data->model, model_data->path, NULL, NULL, model_data->label, model_data->recurse, collector->type);
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
  guint recurse = 0;

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
  recurse = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(model), RECURSE_DATA));

  changed = FALSE;
  for (i = position; i < position + added; i++)
    {
      GMenuModel *link;
      gchar *label = NULL;
      gchar *action_namespace = NULL;
      gchar *action = NULL;
      gchar *accel = NULL;
      gchar *toolbar = NULL;
      gchar *description = NULL;
      gchar *keywords = NULL;
      HudItem *item = NULL;


      g_menu_model_get_item_attribute (model, i, "action-namespace", "s", &action_namespace);
      g_menu_model_get_item_attribute (model, i, G_MENU_ATTRIBUTE_ACTION, "s", &action);
      g_menu_model_get_item_attribute (model, i, G_MENU_ATTRIBUTE_LABEL, "s", &label);
      g_menu_model_get_item_attribute (model, i, "accel", "s", &accel);
      g_menu_model_get_item_attribute (model, i, "hud-toolbar-item", "s", &toolbar);
      g_menu_model_get_item_attribute (model, i, "description", "s", &description);
      g_menu_model_get_item_attribute (model, i, "keywords", "s", &keywords);

      accel = format_accel_for_users(accel);

      /* Check if this is an action.  Here's where we may end up
       * creating a HudItem.
       */
      if (action && label)
        {
          GVariant *target;

          target = g_menu_model_get_item_attribute_value (model, i, G_MENU_ATTRIBUTE_TARGET, NULL);

          item = hud_model_item_new (collector, context, label, action, accel, description, keywords, target, toolbar);

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
          hud_menu_model_collector_add_model_internal (collector, link, NULL, context, action_namespace, label, recurse, collector->type);
          g_object_unref (link);
        }

      if ((link = g_menu_model_get_item_link (model, i, G_MENU_LINK_SUBMENU)))
        {
          hud_menu_model_collector_add_model_internal (collector, link, NULL, context, action_namespace, label, recurse - 1, collector->type);
          if (item != NULL && recurse <= 1)
            {
              hud_model_item_set_submenu((HudModelItem *)item, link, collector->base_export_path);
            }
          g_object_unref (link);
        }

      g_free (action_namespace);
      g_free (action);
      g_free (label);
      g_free (accel);
      g_free (toolbar);
      g_free (description);
      g_free (keywords);
    }

  if (changed)
    hud_source_changed (HUD_SOURCE (collector));
}

/* Unexport a menu and unref the bus we kept */
static void
unexport_menu (gpointer user_data)
{
	exported_menu_id_t * idt = (exported_menu_id_t *)user_data;

	g_debug("Unexport: %s", idt->path);

	g_dbus_connection_unexport_menu_model(idt->bus, idt->id);
	g_object_unref(idt->bus);

	g_clear_object(&idt->export);
	g_free(idt->path);

	g_free(user_data);
	return;
}

static void
hud_menu_model_collector_add_model_internal (HudMenuModelCollector *collector,
                                             GMenuModel            *model,
                                             const gchar           *path,
                                             HudMenuModelContext   *parent_context,
                                             const gchar           *action_namespace,
                                             const gchar           *label,
                                             guint                  recurse,
                                             HudSourceItemType      type)
{
  g_return_if_fail(G_IS_MENU_MODEL(model));

  collector->type = type;

  /* We don't want to parse this one, just export it so that
     the UI could use it if needed */
  if (recurse == 0 && collector->base_export_path != NULL) {
	/* Build a struct for all the info we need */
	exported_menu_id_t * idt = g_new0(exported_menu_id_t, 1);
	idt->bus = g_object_ref(collector->session);

    /* Create the exported model */
    idt->export = g_menu_new();
    GMenuItem * item = g_menu_item_new_submenu("Root Export", model);
    g_menu_append_item(idt->export, item);
    g_object_unref(item);

	/* Export */
    gchar * menu_path = g_strdup_printf("%s/menu%X", collector->base_export_path, GPOINTER_TO_UINT(model));
    g_debug("Exporting menu model: %s", menu_path);
    idt->id = g_dbus_connection_export_menu_model(idt->bus, menu_path, G_MENU_MODEL(idt->export), NULL);

    /* All the callers of this function assume that we take the responsibility
     * to manage the lifecycle of the model. Or in other words, HudMenuModelCollector
     * takes the responsibility.
     *
     * g_dbus_connection_export_menu_model() increases the reference count of the given
     * model and the model is not disposed before it's unexported.
     *
     * By using g_object_set_data_full() we guarantee that the model gets unexported
     * and cleaned up when the collector disposes it self.
     *
     * menu_path is simply used as a unique key to store the idt in collector GObject.
     * There is no need to retrieve the idt using g_object_get_data().
     *
     * The reason that it can't be on the model is because the model is referenced
     * by the exported item.  So the model will never be free'd until the export is
     * first.
     */
    g_object_set_data_full(G_OBJECT(collector), menu_path, idt, unexport_menu);

    return;
  }

  gint n_items;

  g_object_set_data (G_OBJECT(model), RECURSE_DATA, GINT_TO_POINTER(recurse));
  g_signal_connect (model, "items-changed", G_CALLBACK (hud_menu_model_collector_model_changed), collector);

  /* Setup the base structure */
  model_data_t * model_data = g_new0(model_data_t, 1);
  model_data->session = collector->session;
  model_data->model = g_object_ref(model);
  model_data->is_hud_aware = FALSE;
  model_data->cancellable = g_cancellable_new();
  model_data->path = g_strdup(path);
  model_data->label = g_strdup(label);
  model_data->recurse = recurse;

  /* Add to our list of models */
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
  model_data_t * info = (model_data_t *)data;
  g_signal_handlers_disconnect_by_func (info->model, hud_menu_model_collector_model_changed, user_data);
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

      if (search_string == NULL && hud_model_item_is_toolbar(HUD_MODEL_ITEM(item))) {
        continue;
      }

      result = hud_result_get_if_matched (item, search_string, collector->penalty);
      if (result)
        append_func(result, user_data);
    }
}

static void
hud_menu_model_collector_list_applications (HudSource    *source,
                                            HudTokenList *search_string,
                                            void        (*append_func) (const gchar *application_id, const gchar *application_icon, HudSourceItemType type, gpointer user_data),
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
        append_func(collector->app_id, collector->icon, collector->type, user_data);
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

/* Go through the items, find those with toolbar fields that are enabled
   and add them to the list */
static void
hud_menu_model_collector_get_toolbar_entries (HudSource * source, GArray * toolbar)
{
  HudMenuModelCollector *collector = HUD_MENU_MODEL_COLLECTOR (source);

  gint i;
  for (i = 0; i < collector->items->len; i++) {
    HudModelItem * item = g_ptr_array_index(collector->items, i);
    if (item->toolbar_item == -1) continue;
    if (!g_action_group_get_action_enabled(G_ACTION_GROUP(item->group), item->action_name)) continue;

    const gchar * nick = hud_client_query_toolbar_items_get_nick(item->toolbar_item);
    int i;
    for (i = 0; i < toolbar->len; i++) {
      if (g_array_index(toolbar, const gchar *, i) == nick) {
        break;
      }
    }

    if (i == toolbar->len) {
      g_array_append_val(toolbar, nick);
    }
  }

  return;
}

static void
hud_menu_model_collector_activate_toolbar (HudSource * source,
                              HudClientQueryToolbarItems titem,
                              GVariant *platform_data)
{
  HudMenuModelCollector *collector = HUD_MENU_MODEL_COLLECTOR (source);

  gint i;
  for (i = 0; i < collector->items->len; i++) {
    HudModelItem * item = g_ptr_array_index(collector->items, i);
    hud_model_item_activate_toolbar(item, titem, platform_data);
  }

  return;
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

  if (collector->muxer_export) {
    g_dbus_connection_unexport_action_group(collector->session, collector->muxer_export);
    collector->muxer_export = 0;
  }

  g_array_free(collector->agroups, TRUE);
  g_slist_free_full (collector->models, model_data_free);
  g_clear_object (&collector->muxer);

  g_object_unref (collector->session);
  g_free (collector->unique_bus_name);
  g_free (collector->app_id);
  g_free (collector->icon);
  g_object_unref (collector->keyword_mapping);

  g_ptr_array_unref (collector->items);

  g_clear_pointer(&collector->base_export_path, g_free);
  g_clear_pointer(&collector->base_models, g_hash_table_destroy);

  G_OBJECT_CLASS (hud_menu_model_collector_parent_class)
    ->finalize (object);
}

/* Disconnect all the signals and unreference the group */
static void
agroup_clear (gpointer data)
{
	action_group_signals_t * sigs = (action_group_signals_t *)data;

	g_signal_handler_disconnect(sigs->group, sigs->action_added);
	g_signal_handler_disconnect(sigs->group, sigs->action_enabled_changed);
	g_signal_handler_disconnect(sigs->group, sigs->action_removed);
	g_signal_handler_disconnect(sigs->group, sigs->action_state_changed);
	g_object_unref(sigs->group);

	sigs->group = NULL;
	sigs->action_added = 0;
	sigs->action_enabled_changed = 0;
	sigs->action_removed = 0;
	sigs->action_state_changed = 0;

	return;
}

static void
hud_menu_model_collector_init (HudMenuModelCollector *collector)
{
  collector->items = g_ptr_array_new_with_free_func (g_object_unref);
  collector->cancellable = g_cancellable_new ();
  collector->muxer = g_action_muxer_new();
  collector->agroups = g_array_new(FALSE, FALSE, sizeof(action_group_signals_t));
  g_array_set_clear_func(collector->agroups, agroup_clear);
  collector->session = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, NULL);
  collector->base_models = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
}

static void
hud_menu_model_collector_iface_init (HudSourceInterface *iface)
{
  iface->use = hud_menu_model_collector_use;
  iface->unuse = hud_menu_model_collector_unuse;
  iface->search = hud_menu_model_collector_search;
  iface->list_applications = hud_menu_model_collector_list_applications;
  iface->get = hud_menu_model_collector_get;
  iface->get_items = hud_menu_model_collector_get_items;
  iface->get_toolbar_entries = hud_menu_model_collector_get_toolbar_entries;
  iface->activate_toolbar = hud_menu_model_collector_activate_toolbar;
  iface->get_app_id = hud_menu_model_collector_get_app_id;
  iface->get_app_icon = hud_menu_model_collector_get_app_icon;
}

static void
hud_menu_model_collector_class_init (HudMenuModelCollectorClass *class)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (class);

  gobject_class->finalize = hud_menu_model_collector_finalize;
}

static void
hud_menu_model_collector_hud_awareness_cb (GObject      *source,
                                           GAsyncResult *result,
                                           gpointer      user_data)
{
  GVariant *reply;

  if (source == NULL)
  {
    g_debug("Callback invoked with null connection");
    return;
  }

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
 * @export_path: the path to export items on dbus
 * @type: the type of items in this collector
 *
 * Create a new #HudMenuModelCollector object
 *
 * Return value: New #HudMenuModelCollector
 */
HudMenuModelCollector *
hud_menu_model_collector_new (const gchar *application_id,
                              const gchar *icon,
                              guint        penalty,
                              const gchar *export_path,
                              HudSourceItemType type)
{
	g_return_val_if_fail(application_id != NULL, NULL);
	g_return_val_if_fail(export_path != NULL, NULL);

	HudMenuModelCollector * collector = g_object_new(HUD_TYPE_MENU_MODEL_COLLECTOR, NULL);

	collector->app_id = g_strdup (application_id);
	collector->icon = g_strdup (icon);
	collector->penalty = penalty;
	collector->base_export_path = g_strdup(export_path);
	collector->type = type;

	collector->keyword_mapping = hud_keyword_mapping_new();
	hud_keyword_mapping_load(collector->keyword_mapping, collector->app_id, DATADIR, GNOMELOCALEDIR);

	GError * error = NULL;
	collector->muxer_export = g_dbus_connection_export_action_group(collector->session,
	                                                                collector->base_export_path,
	                                                                G_ACTION_GROUP(collector->muxer),
	                                                                &error);

	if (error != NULL) {
		g_warning("Unable to export action group: %s", error->message);
		g_error_free(error);
	}

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
                                     AbstractWindow  *window)
{
  g_return_if_fail(HUD_IS_MENU_MODEL_COLLECTOR(collector));

#ifdef HAVE_BAMF
  gchar *unique_bus_name;
  gchar *application_object_path;
  gchar *window_object_path;
  gchar *app_menu_object_path;
  gchar *menubar_object_path;
  gchar *unity_object_path = NULL;

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
  unity_object_path = bamf_window_get_utf8_prop (window, "_UNITY_OBJECT_PATH");

  if (app_menu_object_path && !g_hash_table_lookup(collector->base_models, app_menu_object_path))
    {
      GDBusMenuModel * app_menu;
      app_menu = g_dbus_menu_model_get (collector->session, collector->unique_bus_name, app_menu_object_path);
      hud_menu_model_collector_add_model_internal (collector, G_MENU_MODEL (app_menu), app_menu_object_path, NULL, NULL, NULL, HUD_MENU_MODEL_DEFAULT_DEPTH, collector->type);
      g_object_unref(app_menu);

      g_debug("Adding menu model: %s", app_menu_object_path);
      g_hash_table_insert(collector->base_models, g_strdup(app_menu_object_path), GINT_TO_POINTER(TRUE));
    }

  if (menubar_object_path && !g_hash_table_lookup(collector->base_models, menubar_object_path))
    {
      GDBusMenuModel * menubar;
      menubar = g_dbus_menu_model_get (collector->session, collector->unique_bus_name, menubar_object_path);
      hud_menu_model_collector_add_model_internal (collector, G_MENU_MODEL (menubar), menubar_object_path, NULL, NULL, NULL, HUD_MENU_MODEL_DEFAULT_DEPTH, collector->type);
      g_object_unref(menubar);

      g_debug("Adding menu model: %s", menubar_object_path);
      g_hash_table_insert(collector->base_models, g_strdup(menubar_object_path), GINT_TO_POINTER(TRUE));
    }

  if (unity_object_path && !g_hash_table_lookup(collector->base_models, unity_object_path))
    {
      GDBusMenuModel * menubar = g_dbus_menu_model_get (collector->session, collector->unique_bus_name, unity_object_path);
      hud_menu_model_collector_add_model_internal (collector, G_MENU_MODEL (menubar), unity_object_path, NULL, NULL, NULL, HUD_MENU_MODEL_DEFAULT_DEPTH, collector->type);
      g_object_unref(menubar);

      g_debug("Adding menu model: %s", unity_object_path);
      g_hash_table_insert(collector->base_models, g_strdup(unity_object_path), GINT_TO_POINTER(TRUE));
    }

  if (application_object_path)
    {
      GDBusActionGroup * ag = g_dbus_action_group_get (collector->session, collector->unique_bus_name, application_object_path);
      hud_menu_model_collector_add_actions(collector, G_ACTION_GROUP(ag), "app");
      g_object_unref(ag);
    }

  if (window_object_path)
    {
      GDBusActionGroup * ag = g_dbus_action_group_get (collector->session, collector->unique_bus_name, window_object_path);
      hud_menu_model_collector_add_actions(collector, G_ACTION_GROUP(ag), "win");
      g_object_unref(ag);
    }

  if (unity_object_path)
    {
      GDBusActionGroup * ag = g_dbus_action_group_get (collector->session, collector->unique_bus_name, unity_object_path);
      hud_menu_model_collector_add_actions(collector, G_ACTION_GROUP(ag), "unity");
      g_object_unref(ag);
    }

  /* when the action groups change, we could end up having items
   * enabled/disabled.  how to deal with that?
   */

  g_free (app_menu_object_path);
  g_free (menubar_object_path);
  g_free (application_object_path);
  g_free (window_object_path);
  g_free (unity_object_path);
#endif

  return;
}

/**
 * hud_menu_model_collector_add_endpoint:
 * @prefix: the title to prefix to all items
 * @bus_name: a D-Bus bus name
 * @menu_path: an object path at the destination given by @bus_name for the menu model
 * @action_path: an object path at the destination given by @bus_name for the actions
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
                                       const gchar *menu_path,
                                       const gchar *action_path)
{
  g_return_if_fail(HUD_IS_MENU_MODEL_COLLECTOR(collector));

  GActionGroup * group = G_ACTION_GROUP(g_dbus_action_group_get (collector->session, bus_name, action_path));
  hud_menu_model_collector_add_actions(collector, group, NULL);
  g_object_unref(group);

  GDBusMenuModel * app_menu = g_dbus_menu_model_get (collector->session, bus_name, menu_path);
  hud_menu_model_collector_add_model(collector, G_MENU_MODEL (app_menu), prefix, HUD_MENU_MODEL_DEFAULT_DEPTH);
  g_object_unref(app_menu);

  return;
}

/**
 * hud_menu_model_collector_add_model:
 * @collector: A #HudMenuModelCollector object
 * @model: Model to add
 * @prefix: (allow none): Text prefix to add to all entries if needed
 * @recurse: Amount of levels to go down the model.
 *
 * Adds a Menu Model to the collector.
 */
void
hud_menu_model_collector_add_model (HudMenuModelCollector * collector, GMenuModel * model, const gchar * prefix, guint recurse)
{
	g_return_if_fail(HUD_IS_MENU_MODEL_COLLECTOR(collector));
	g_return_if_fail(G_IS_MENU_MODEL(model));

	return hud_menu_model_collector_add_model_internal(collector, model, NULL, NULL, NULL, prefix, recurse, collector->type);
}

/* When the action groups change let's pass that up as a change to
   this source. */
static void
action_group_changed (GActionGroup * group, const gchar * action, gpointer user_data)
{
	hud_source_changed(HUD_SOURCE(user_data));
}

static void
action_group_enabled (GActionGroup * group, const gchar * action, gboolean enabled, gpointer user_data)
{
	hud_source_changed(HUD_SOURCE(user_data));
}

static void
action_group_state (GActionGroup * group, const gchar * action, GVariant * value, gpointer user_data)
{
	hud_source_changed(HUD_SOURCE(user_data));
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

	action_group_signals_t sigs;

	sigs.group = g_object_ref(group);
	sigs.action_added = g_signal_connect(G_OBJECT(group), "action-added", G_CALLBACK(action_group_changed), collector);
	sigs.action_enabled_changed = g_signal_connect(G_OBJECT(group), "action-enabled-changed", G_CALLBACK(action_group_enabled), collector);
	sigs.action_removed = g_signal_connect(G_OBJECT(group), "action-removed", G_CALLBACK(action_group_changed), collector);
	sigs.action_state_changed = g_signal_connect(G_OBJECT(group), "action-state-changed", G_CALLBACK(action_group_state), collector);

	g_array_append_val(collector->agroups, sigs);

	g_action_muxer_insert(collector->muxer, prefix, group);

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
hud_menu_model_collector_get_items (HudSource * source)
{
	g_return_val_if_fail(HUD_IS_MENU_MODEL_COLLECTOR(source), NULL);
	HudMenuModelCollector * mcollector = HUD_MENU_MODEL_COLLECTOR(source);

	GList * retval = NULL;
	int i;
	for (i = 0; i < mcollector->items->len; i++) {
		retval = g_list_prepend(retval, g_object_ref(g_ptr_array_index(mcollector->items, i)));
	}

	return retval;
}

/**
 * hud_menu_model_collector_get_app_id:
 * @collector: A #HudMenuModelCollector
 *
 * The ID of the application here
 *
 * Return value: Application ID
 */
static const gchar *
hud_menu_model_collector_get_app_id (HudSource * collector)
{
	g_return_val_if_fail(HUD_IS_MENU_MODEL_COLLECTOR(collector), NULL);
	return HUD_MENU_MODEL_COLLECTOR(collector)->app_id;
}

/**
 * hud_menu_model_collector_get_app_icon:
 * @collector: A #HudMenuModelCollector
 *
 * The icon of the application here
 *
 * Return value: Application icon
 */
static const gchar *
hud_menu_model_collector_get_app_icon (HudSource * collector)
{
	g_return_val_if_fail(HUD_IS_MENU_MODEL_COLLECTOR(collector), NULL);
	return HUD_MENU_MODEL_COLLECTOR(collector)->icon;
}
