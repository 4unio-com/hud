/*
 * Copyright © 2013 Canonical Ltd.
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
 * Author: Ted Gould <ted@canonical.com>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "application-source-context.h"
#include "hudsource.h"
#include "huddbusmenucollector.h"
#include "hudmenumodelcollector.h"

/* GObject Basis funcs */
static void hud_application_source_context_class_init (HudApplicationSourceContextClass *klass);
static void hud_application_source_context_init       (HudApplicationSourceContext *self);
static void hud_application_source_context_dispose    (GObject *object);
static void hud_application_source_context_finalize   (GObject *object);

/* Source Interface Funcs */
static void source_iface_init                         (HudSourceInterface *      iface);
static void source_use                                (HudSource *               hud_source);
static void source_unuse                              (HudSource *               hud_source);
static void source_search                             (HudSource *               hud_source,
                                                       HudTokenList *            search_string,
                                                       void                    (*append_func) (HudResult * result, gpointer user_data),
                                                       gpointer                  user_data);
static void source_get_toolbar_entries                (HudSource *               hud_source,
                                                       GArray *                  toolbar);
static void source_activate_toolbar                   (HudSource *               hud_source,
                                                       HudClientQueryToolbarItems  item,
                                                       GVariant                 *platform_data);
static GList * source_get_items                       (HudSource *               object);

/* #define object building */
G_DEFINE_TYPE_WITH_CODE (HudApplicationSourceContext, hud_application_source_context, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (HUD_TYPE_SOURCE, source_iface_init))

/* Privates */
struct _HudApplicationSourceContextPrivate {
	/* Identification */
	guint32 window_id;
	gchar * context_id;
	gchar * app_id;
	gchar * icon;
	gchar * app_path;

	/* Generated */
	gchar * export_path;

	/* Collectors */
	HudDbusmenuCollector * window_menus_dbus;
	HudMenuModelCollector * model_collector;
};

#define HUD_APPLICATION_SOURCE_CONTEXT_GET_PRIVATE(o) \
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), HUD_TYPE_APPLICATION_SOURCE_CONTEXT, HudApplicationSourceContextPrivate))

/* Initialize the class data */
static void
hud_application_source_context_class_init (HudApplicationSourceContextClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (HudApplicationSourceContextPrivate));

	object_class->dispose = hud_application_source_context_dispose;
	object_class->finalize = hud_application_source_context_finalize;

	return;
}

/* Initialize the instance data */
static void
hud_application_source_context_init (HudApplicationSourceContext *self)
{
	self->priv = HUD_APPLICATION_SOURCE_CONTEXT_GET_PRIVATE(self);

	return;
}

/* Free references */
static void
hud_application_source_context_dispose (GObject *object)
{
	HudApplicationSourceContext * context = HUD_APPLICATION_SOURCE_CONTEXT(object);

	g_clear_object(&context->priv->window_menus_dbus);
	g_clear_object(&context->priv->model_collector);

	G_OBJECT_CLASS (hud_application_source_context_parent_class)->dispose (object);
	return;
}

/* Free memory */
static void
hud_application_source_context_finalize (GObject *object)
{
	HudApplicationSourceContext * context = HUD_APPLICATION_SOURCE_CONTEXT(object);

	g_clear_pointer(&context->priv->context_id, g_free);
	g_clear_pointer(&context->priv->app_id, g_free);
	g_clear_pointer(&context->priv->icon, g_free);
	g_clear_pointer(&context->priv->app_path, g_free);

	g_clear_pointer(&context->priv->export_path, g_free);

	G_OBJECT_CLASS (hud_application_source_context_parent_class)->finalize (object);
	return;
}

/* Initialize the source interface */
static void
source_iface_init (HudSourceInterface * iface)
{
	iface->use = source_use;
	iface->unuse = source_unuse;
	iface->search = source_search;
	iface->get_toolbar_entries = source_get_toolbar_entries;
	iface->activate_toolbar = source_activate_toolbar;
	iface->get_items = source_get_items;

	return;
}

/* Mark all the sources used */
static void
source_use (HudSource * hud_source)
{
	g_return_if_fail(HUD_IS_APPLICATION_SOURCE_CONTEXT(hud_source));
	HudApplicationSourceContext * context = HUD_APPLICATION_SOURCE_CONTEXT(hud_source);

	if (context->priv->window_menus_dbus != NULL) {
		hud_source_use(HUD_SOURCE(context->priv->window_menus_dbus));
	}

	if (context->priv->model_collector != NULL) {
		hud_source_use(HUD_SOURCE(context->priv->model_collector));
	}

	return;
}

/* Mark all the sources unused */
static void
source_unuse (HudSource * hud_source)
{
	g_return_if_fail(HUD_IS_APPLICATION_SOURCE_CONTEXT(hud_source));
	HudApplicationSourceContext * context = HUD_APPLICATION_SOURCE_CONTEXT(hud_source);

	if (context->priv->window_menus_dbus != NULL) {
		hud_source_unuse(HUD_SOURCE(context->priv->window_menus_dbus));
	}

	if (context->priv->model_collector != NULL) {
		hud_source_unuse(HUD_SOURCE(context->priv->model_collector));
	}

	return;
}

/* Search all of our internal sources */
static void
source_search (HudSource * hud_source, HudTokenList * search_string, void (*append_func) (HudResult * result, gpointer user_data), gpointer user_data)
{
	g_return_if_fail(HUD_IS_APPLICATION_SOURCE_CONTEXT(hud_source));
	HudApplicationSourceContext * context = HUD_APPLICATION_SOURCE_CONTEXT(hud_source);

	if (context->priv->window_menus_dbus != NULL) {
		hud_source_search(HUD_SOURCE(context->priv->window_menus_dbus), search_string, append_func, user_data);
	}

	if (context->priv->model_collector != NULL) {
		hud_source_search(HUD_SOURCE(context->priv->model_collector), search_string, append_func, user_data);
	}

	return;
}

/* Get Toolbar Entries */
static void
source_get_toolbar_entries (HudSource * hud_source, GArray * toolbar)
{
	g_return_if_fail(HUD_IS_APPLICATION_SOURCE_CONTEXT(hud_source));
	HudApplicationSourceContext * context = HUD_APPLICATION_SOURCE_CONTEXT(hud_source);

	if (context->priv->model_collector != NULL) {
		hud_source_get_toolbar_entries(HUD_SOURCE(context->priv->model_collector), toolbar);
	}

	return;
}

/* Pass down toolbar activations */
static void
source_activate_toolbar (HudSource * hud_source, HudClientQueryToolbarItems item, GVariant * platform_data)
{
	g_return_if_fail(HUD_IS_APPLICATION_SOURCE_CONTEXT(hud_source));
	HudApplicationSourceContext * context = HUD_APPLICATION_SOURCE_CONTEXT(hud_source);

	if (context->priv->window_menus_dbus != NULL) {
		hud_source_activate_toolbar(HUD_SOURCE(context->priv->window_menus_dbus), item, platform_data);
	}

	if (context->priv->model_collector != NULL) {
		hud_source_activate_toolbar(HUD_SOURCE(context->priv->model_collector), item, platform_data);
	}

	return;
}

/* Collect all the items */
static GList *
source_get_items (HudSource * object)
{
	g_return_val_if_fail(HUD_IS_APPLICATION_SOURCE_CONTEXT(object), NULL);
	HudApplicationSourceContext * app = HUD_APPLICATION_SOURCE_CONTEXT(object);

	GList * retval = NULL;

	if (app->priv->model_collector != NULL) {
		retval = g_list_concat(hud_source_get_items(HUD_SOURCE(app->priv->model_collector)), retval);
	}

	if (app->priv->window_menus_dbus != NULL) {
		retval = g_list_concat(hud_source_get_items(HUD_SOURCE(app->priv->window_menus_dbus)), retval);
	}

	return retval;
}

/**
 * hud_application_source_context_new:
 * @window_id: Window associated with the data
 * @context_id: (allow-none): The context of the data
 *
 * Builds an structure to track the items for a given context in a
 * given window.
 *
 * Return value: (transfer full): A new #HudApplicationSourceContext object
 */
HudApplicationSourceContext *
hud_application_source_context_new (guint32 window_id, const gchar * context_id, const gchar * app_id, const gchar * icon, const gchar * app_path)
{
	g_return_val_if_fail(app_id != NULL && app_id[0] != '\0', NULL);
	g_return_val_if_fail(context_id == NULL || context_id[0] != '\0', NULL);
	g_return_val_if_fail(g_variant_is_object_path(app_path), NULL);

	HudApplicationSourceContext * context = g_object_new(HUD_TYPE_APPLICATION_SOURCE_CONTEXT, NULL);

	context->priv->window_id = window_id;
	context->priv->context_id = g_strdup(context_id);
	context->priv->app_id = g_strdup(app_id);
	context->priv->icon = g_strdup(icon);
	context->priv->app_path = g_strdup(app_path);

	if (context_id != NULL) {
		context->priv->export_path = g_strdup_printf("%s/window_%d/context_%s", app_path, window_id, context_id);
	} else {
		context->priv->export_path = g_strdup_printf("%s/window_%d/context_none", app_path, window_id);
	}

	return context;
}

/**
 * hud_application_source_context_get_window_id:
 * @context: The #HudApplicationSourceContext to look into
 *
 * Gets the window ID this context was built with.
 *
 * Return value: A window ID or #HUD_APPLICATION_SOURCE_CONTEXT_ALL_WINDOWS
 */
guint32
hud_application_source_context_get_window_id (HudApplicationSourceContext * context)
{
	g_return_val_if_fail(HUD_IS_APPLICATION_SOURCE_CONTEXT(context), 0);

	return context->priv->window_id;
}

/**
 * hud_application_source_context_get_context_id:
 * @context: The #HudApplicationSourceContext to look into
 *
 * Gets the context ID this context was built with.
 *
 * Return value: A context ID or #HUD_APPLICATION_SOURCE_CONTEXT_NO_CONTEXT
 */
const gchar *
hud_application_source_context_get_context_id (HudApplicationSourceContext * context)
{
	g_return_val_if_fail(HUD_IS_APPLICATION_SOURCE_CONTEXT(context), NULL);

	return context->priv->context_id;
}

/* Check to ensure we have a menu model collector, if not, build
   us one. */
static inline void
check_for_menu_model (HudApplicationSourceContext * context)
{
	if (context->priv->model_collector == NULL) {
		context->priv->model_collector = hud_menu_model_collector_new(context->priv->app_id, context->priv->icon, 0, context->priv->export_path, HUD_SOURCE_ITEM_TYPE_BACKGROUND_APP);
	}
	g_return_if_fail(HUD_IS_MENU_MODEL_COLLECTOR(context->priv->model_collector));

	return;
}

/**
 * hud_application_source_context_add_action_group:
 * @context: The #HudApplicationSourceContext to look into
 * @group: Action group to add
 *
 * Adds an action group to the items indexed by this context.
 */
void
hud_application_source_context_add_action_group (HudApplicationSourceContext * context, GActionGroup * group, const gchar * prefix)
{
	g_return_if_fail(HUD_IS_APPLICATION_SOURCE_CONTEXT(context));
	g_return_if_fail(G_IS_ACTION_GROUP(group));
	check_for_menu_model(context);

	hud_menu_model_collector_add_actions(context->priv->model_collector, group, prefix);

	return;
}

/**
 * hud_application_source_context_add_model:
 * @context: The #HudApplicationSourceContext to look into
 * @model: menu model to add
 *
 * Adds a model to the items indexed by this context.
 */
void
hud_application_source_context_add_model (HudApplicationSourceContext * context, GMenuModel * model, HudApplicationSourceContextModelType type)
{
	g_return_if_fail(HUD_IS_APPLICATION_SOURCE_CONTEXT(context));
	g_return_if_fail(G_IS_MENU_MODEL(model));
	check_for_menu_model(context);

	gint depth = HUD_MENU_MODEL_DEFAULT_DEPTH;
	if (type == HUD_APPLICATION_SOURCE_CONTEXT_MODEL_DBUS) {
		depth = 1;
	}

	hud_menu_model_collector_add_model(context->priv->model_collector, model, NULL, depth);

	return;
}

/**
 * hud_application_source_context_add_window:
 * @context: The #HudApplicationSourceContext to look into
 * @window: window to add
 *
 * Adds a window to the items indexed by this context.
 */
void
hud_application_source_context_add_window (HudApplicationSourceContext * context, AbstractWindow * window)
{
	g_return_if_fail(HUD_IS_APPLICATION_SOURCE_CONTEXT(context));
	check_for_menu_model(context);

	hud_menu_model_collector_add_window(context->priv->model_collector, window);

	if (context->priv->window_menus_dbus == NULL) {
		context->priv->window_menus_dbus = hud_dbusmenu_collector_new_for_window(window, context->priv->app_id, context->priv->icon, HUD_SOURCE_ITEM_TYPE_BACKGROUND_APP);
	} else {
		guint32 oldid = hud_dbusmenu_collector_get_xid(context->priv->window_menus_dbus);
		guint32 newid = abstract_window_get_id(window);

		if (oldid != newid) {
			g_warning("Adding a second DBus Menu window (%X) to a context.  Current window (%X).", newid, oldid);
		}
	}

	return;
}
