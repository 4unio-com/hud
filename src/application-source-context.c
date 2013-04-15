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

/* #define object building */
G_DEFINE_TYPE_WITH_CODE (HudApplicationSourceContext, hud_application_source_context, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (HUD_TYPE_SOURCE, source_iface_init))

/* Privates */
struct _HudApplicationSourceContextPrivate {
	/* Identification */
	guint32 window_id;
	gchar * context_id;

	/* Collectors */
	HudDbusmenuCollector * window_menus_dbus;
	GPtrArray * model_sources;
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

	/* Starting out with 4, seems an unlikely case that we'll use more than that, but we
	   want to avoid frequent reallocation. */
	self->priv->model_sources = g_ptr_array_new_full(4, g_object_unref);
	return;
}

/* Free references */
static void
hud_application_source_context_dispose (GObject *object)
{
	HudApplicationSourceContext * context = HUD_APPLICATION_SOURCE_CONTEXT(object);

	g_clear_object(&context->priv->window_menus_dbus);
	g_clear_pointer(&context->priv->model_sources, g_ptr_array_unref);

	G_OBJECT_CLASS (hud_application_source_context_parent_class)->dispose (object);
	return;
}

/* Free memory */
static void
hud_application_source_context_finalize (GObject *object)
{
	HudApplicationSourceContext * context = HUD_APPLICATION_SOURCE_CONTEXT(object);

	g_clear_pointer(&context->priv->context_id, g_free);

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

	int i;
	for (i = 0; i < context->priv->model_sources->len; i++) {
		hud_source_use(HUD_SOURCE(g_ptr_array_index(context->priv->model_sources, i)));
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

	int i;
	for (i = 0; i < context->priv->model_sources->len; i++) {
		hud_source_unuse(HUD_SOURCE(g_ptr_array_index(context->priv->model_sources, i)));
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

	int i;
	for (i = 0; i < context->priv->model_sources->len; i++) {
		hud_source_search(HUD_SOURCE(g_ptr_array_index(context->priv->model_sources, i)), search_string, append_func, user_data);
	}

	return;
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
hud_application_source_context_new (guint32 window_id, const gchar * context_id)
{
	HudApplicationSourceContext * context = g_object_new(HUD_TYPE_APPLICATION_SOURCE_CONTEXT, NULL);

	context->priv->window_id = window_id;
	context->priv->context_id = g_strdup(context_id);

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

/**
 * hud_application_source_context_add_action_group:
 * @context: The #HudApplicationSourceContext to look into
 * @group: Action group to add
 *
 * Adds an action group to the items indexed by this context.
 */
void
hud_application_source_context_add_action_group (HudApplicationSourceContext * context, GActionGroup * group)
{
	g_return_if_fail(HUD_IS_APPLICATION_SOURCE_CONTEXT(context));
	g_return_if_fail(G_IS_ACTION_GROUP(group));

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
hud_application_source_context_add_model (HudApplicationSourceContext * context, GMenuModel * model)
{
	g_return_if_fail(HUD_IS_APPLICATION_SOURCE_CONTEXT(context));
	g_return_if_fail(G_IS_MENU_MODEL(model));

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

	return;
}
