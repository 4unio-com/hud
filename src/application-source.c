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
 * Author: Ted Gould <ted@canonical.com>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "application-source.h"
#include "hudsource.h"
#include "app-iface.h"
#include "hudmenumodelcollector.h"
#include "huddbusmenucollector.h"
#include "hudsourcelist.h"

struct _HudApplicationSourcePrivate {
	gchar * app_id;
	gchar * path;
	AppIfaceComCanonicalHudApplication * skel;

	BamfApplication * bamf_app;

	guint32 focused_window;

	GHashTable * windows;
};

#define HUD_APPLICATION_SOURCE_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), HUD_TYPE_APPLICATION_SOURCE, HudApplicationSourcePrivate))

static void hud_application_source_class_init (HudApplicationSourceClass * klass);
static void hud_application_source_init       (HudApplicationSource *      self);
static void hud_application_source_dispose    (GObject *                   object);
static void hud_application_source_finalize   (GObject *                   object);
static void source_iface_init                 (HudSourceInterface *        iface);
static void source_use                        (HudSource *                 hud_source);
static void source_unuse                      (HudSource *                 hud_source);
static void source_search                     (HudSource *                 hud_source,
                                               HudTokenList *              search_string,
                                               void                      (*append_func) (HudResult * result, gpointer user_data),
                                               gpointer                    user_data);
static gboolean dbus_add_sources              (AppIfaceComCanonicalHudApplication * skel,
                                               GDBusMethodInvocation *     invocation,
                                               GVariant *                  actions,
                                               GVariant *                  descs,
                                               gpointer                    user_data);

G_DEFINE_TYPE_WITH_CODE (HudApplicationSource, hud_application_source, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (HUD_TYPE_SOURCE, source_iface_init))

/* Class Init */
static void
hud_application_source_class_init (HudApplicationSourceClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (HudApplicationSourcePrivate));

	object_class->dispose = hud_application_source_dispose;
	object_class->finalize = hud_application_source_finalize;

	return;
}

/* Intialized the source interface */
static void
source_iface_init (HudSourceInterface *iface)
{
	iface->use = source_use;
	iface->unuse = source_unuse;
	iface->search = source_search;

	return;
}

/* Instance Init */
static void
hud_application_source_init (HudApplicationSource *self)
{
	self->priv = HUD_APPLICATION_SOURCE_GET_PRIVATE(self);

	self->priv->windows = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, g_object_unref);

	return;
}

/* Clean up references */
static void
hud_application_source_dispose (GObject *object)
{
	HudApplicationSource * self = HUD_APPLICATION_SOURCE(object);

	g_clear_pointer(&self->priv->windows, g_hash_table_unref);

	if (self->priv->skel != NULL) {
		g_dbus_interface_skeleton_unexport(G_DBUS_INTERFACE_SKELETON(self->priv->skel));
		g_clear_object(&self->priv->skel);
	}

	g_clear_object(&self->priv->bamf_app);

	G_OBJECT_CLASS (hud_application_source_parent_class)->dispose (object);
	return;
}

/* Free memory */
static void
hud_application_source_finalize (GObject *object)
{
	HudApplicationSource * self = HUD_APPLICATION_SOURCE(object);

	g_clear_pointer(&self->priv->app_id, g_free);
	g_clear_pointer(&self->priv->path, g_free);

	G_OBJECT_CLASS (hud_application_source_parent_class)->finalize (object);
	return;
}

/* Source interface using this source */
static void
source_use (HudSource *hud_source)
{

	return;
}

/* Source interface unusing this source */
static void
source_unuse (HudSource *hud_source)
{

	return;
}

/* Search this source */
static void
source_search (HudSource *     hud_source,
               HudTokenList *  search_string,
               void          (*append_func) (HudResult * result, gpointer user_data),
               gpointer        user_data)
{
	HudApplicationSource * app = HUD_APPLICATION_SOURCE(hud_source);
	HudSource * collector = g_hash_table_lookup(app->priv->windows, GINT_TO_POINTER(app->priv->focused_window));

	if (collector == NULL) {
		return;
	}

	hud_source_search(collector, search_string, append_func, user_data);
	return;
}

HudApplicationSource *
hud_application_source_new_for_app (BamfApplication * bapp)
{
	gchar * id = hud_application_source_bamf_app_id(bapp);
	if (id == NULL) {
		return NULL;
	}

	HudApplicationSource * source = hud_application_source_new_for_id(id);
	g_free(id);

	source->priv->bamf_app = g_object_ref(bapp);

	return source;
}

HudApplicationSource *
hud_application_source_new_for_id (const gchar * id)
{
	HudApplicationSource * source = g_object_new(HUD_TYPE_APPLICATION_SOURCE, NULL);

	source->priv->app_id = g_strdup(id);

	source->priv->skel = app_iface_com_canonical_hud_application_skeleton_new();
	g_signal_connect(G_OBJECT(source->priv->skel), "handle-add-sources", G_CALLBACK(dbus_add_sources), source);

	gchar * app_id_clean = g_strdup(id);
	gchar * app_id_cleanp;
	for (app_id_cleanp = app_id_clean; app_id_cleanp[0] != '\0'; app_id_cleanp++) {
		if (!g_ascii_isalnum(app_id_cleanp[0])) {
			app_id_cleanp[0] = '_';
		}
	}
	source->priv->path = g_strdup_printf("/com/canonical/hud/applications/%s", app_id_clean);

	int i = 0;
	while (!g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(source->priv->skel),
	                                 g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, NULL),
	                                 source->priv->path,
	                                 NULL)) {
		g_free(source->priv->path);
		source->priv->path = g_strdup_printf("/com/canonical/hud/applications/%s_%d", app_id_clean, ++i);
	}

	g_debug("Application ('%s') path: %s", id, source->priv->path);
	g_free(app_id_clean);

	return source;
}

/* Get the collectors if we need them */
static void
get_collectors (HudApplicationSource * app, guint32 xid, const gchar * appid, HudDbusmenuCollector ** dcollector, HudMenuModelCollector ** mcollector)
{
	HudSourceList * collector_list = g_hash_table_lookup(app->priv->windows, GINT_TO_POINTER(xid));
	if (collector_list == NULL) {
		collector_list = hud_source_list_new();
		g_hash_table_insert(app->priv->windows, GINT_TO_POINTER(xid), collector_list);
	}

	HudMenuModelCollector * mm_collector = NULL;
	HudDbusmenuCollector * dm_collector = NULL;
	GSList * sources = hud_source_list_get_list(collector_list);
	GSList * source;
	for (source = sources; source != NULL; source = g_slist_next(source)) {
		if (HUD_IS_MENU_MODEL_COLLECTOR(source->data)) {
			mm_collector = HUD_MENU_MODEL_COLLECTOR(source->data);
		}
		if (HUD_IS_DBUSMENU_COLLECTOR(source->data)) {
			dm_collector = HUD_DBUSMENU_COLLECTOR(source->data);
		}
	}
	if (mm_collector == NULL) {
		mm_collector = hud_menu_model_collector_new(appid, NULL, 0);

		if (mm_collector != NULL) {
			hud_source_list_add(collector_list, HUD_SOURCE(mm_collector));
		}
	}

	if (dcollector != NULL) {
		*dcollector = dm_collector;
	}
	if (mcollector != NULL) {
		*mcollector = mm_collector;
	}

	return;
}

/* Respond to the DBus function to add sources */
static gboolean
dbus_add_sources (AppIfaceComCanonicalHudApplication * skel, GDBusMethodInvocation * invocation, GVariant * actions, GVariant * descs, gpointer user_data)
{
	HudApplicationSource * app = HUD_APPLICATION_SOURCE(user_data);
	GDBusConnection * session = g_dbus_method_invocation_get_connection(invocation);
	const gchar * sender = g_dbus_method_invocation_get_sender(invocation);

	GVariantIter action_iter;
	g_variant_iter_init(&action_iter, actions);

	GVariant * id = NULL;
	gchar * prefix = NULL;
	gchar * object = NULL;

	while (g_variant_iter_loop(&action_iter, "(vso)", &id, &prefix, &object)) {
		g_debug("Adding prefix '%s' at path: %s", prefix, object);

		guint32 idn = g_variant_get_int32(id);

		HudMenuModelCollector * collector = NULL;
		get_collectors(app, idn, app->priv->app_id, NULL, &collector);
		if (collector == NULL) continue;

		GDBusActionGroup * ag = g_dbus_action_group_get(session, sender, object);

		hud_menu_model_collector_add_actions(collector, G_ACTION_GROUP(ag), prefix);
	}

	GVariantIter desc_iter;
	g_variant_iter_init(&desc_iter, descs);

	while (g_variant_iter_loop(&desc_iter, "(vo)", &id, &object)) {
		g_debug("Adding descriptions: %s", object);

		guint32 idn = g_variant_get_int32(id);

		HudMenuModelCollector * collector = NULL;
		get_collectors(app, idn, app->priv->app_id, NULL, &collector);
		if (collector == NULL) continue;

		GDBusMenuModel * model = g_dbus_menu_model_get(session, sender, object);

		hud_menu_model_collector_add_model(collector, G_MENU_MODEL(model), NULL);
	}

	g_dbus_method_invocation_return_value(invocation, NULL);
	return TRUE;
}

/**
 * hud_application_source_bamf_app_id:
 * @app: A #BamfApplication object
 *
 * Check to see if we don't have any collectors left.
 *
 * Return value: The state of the source
 */
gboolean
hud_application_source_is_empty (HudApplicationSource * app)
{
	g_return_val_if_fail(HUD_IS_APPLICATION_SOURCE(app), TRUE);

	return (g_hash_table_size(app->priv->windows) == 0);
}

/**
 * hud_application_source_bamf_app_id:
 * @bapp: A #BamfApplication object
 *
 * A little helper function to genereate a constant app ID out of
 * BAMF Application objects.  Putting this here as it seems to make
 * the most sense, but isn't really part of the object.
 *
 * Return value: (transfer full): ID for the application
 */
gchar *
hud_application_source_bamf_app_id (BamfApplication * bapp)
{
	g_return_val_if_fail(BAMF_IS_APPLICATION(bapp), NULL);

	const gchar * desktop_file = bamf_application_get_desktop_file(bapp);
	if (desktop_file == NULL) {
		/* Some apps might not be identifiable.  Eh, don't care then */
		return NULL;
	}

	gchar * basename = g_path_get_basename(desktop_file);
	if (basename == NULL || basename[0] == '\0' || !g_str_has_suffix(basename, ".desktop")) {
		/* Check to make sure it's not NULL and it returns a desktop file */
		g_free(basename);
		return NULL;
	}

	/* This is probably excessively clever, but I like it.  Basically we find
	   the last instance of .desktop and put the null there.  For all practical
	   purposes this is a NULL terminated string of the first part of the dekstop
	   file name */
	g_strrstr(basename, ".desktop")[0] = '\0';

	return basename;
}

/**
 * hud_application_source_focus:
 * @app: A #HudApplicationSource object
 * @bapp: The #BamfApplication representing this @app
 * @window: The #BamfWindow that has focus
 *
 * Tells the application source that focus has changed to it.  This
 * means that we can do things like figure out what window has focus
 * and make sure we're all good.
 */
void
hud_application_source_focus (HudApplicationSource * app, BamfApplication * bapp, BamfWindow * window)
{
	g_return_if_fail(HUD_IS_APPLICATION_SOURCE(app));
	g_return_if_fail(BAMF_IS_APPLICATION(bapp));

	if (app->priv->bamf_app == NULL) {
		app->priv->bamf_app = g_object_ref(bapp);
	}

	g_return_if_fail(app->priv->bamf_app == bapp);

	/* TODO: Fill in */
	hud_application_source_add_window(app, window);
	app->priv->focused_window = bamf_window_get_xid(window);

	return;
}

/**
 * hud_application_source_get_path:
 * @app: A #HudApplicationSource object
 *
 * Get the object path for this source on DBus
 *
 * Return value: The path as a string
 */
const gchar *
hud_application_source_get_path (HudApplicationSource * app)
{
	g_return_val_if_fail(HUD_IS_APPLICATION_SOURCE(app), NULL);

	return app->priv->path;
}

/**
 * hud_application_source_get_id:
 * @app: A #HudApplicationSource object
 *
 * Get the app id for this source on DBus
 *
 * Return value: The id as a string
 */
const gchar *
hud_application_source_get_id (HudApplicationSource * app)
{
	g_return_val_if_fail(HUD_IS_APPLICATION_SOURCE(app), NULL);

	return app->priv->app_id;
}

typedef struct _window_info_t window_info_t;
struct _window_info_t {
	HudApplicationSource * source;  /* Not a ref */
	BamfWindow * window;            /* Not a ref */
	guint32 xid;                    /* Can't be a ref */
};

/* When I window gets destroyed we want to clean up it's collectors
   and all that jazz. */
static void
window_destroyed (gpointer data, GObject * old_address)
{
	window_info_t * window_info = (window_info_t *)data;

	window_info->window = NULL;

	g_hash_table_remove(window_info->source->priv->windows, GINT_TO_POINTER(window_info->xid));
	/* NOTE: DO NOT use the window_info after this point as
	   it may be free'd by the remove above. */

	return;
}

/* If the collector gets free'd first we need to deallocate the memory
   and make sure we don't keep the weak reference. */
static void
free_window_info (gpointer data)
{
	window_info_t * window_info = (window_info_t *)data;

	if (window_info->window != NULL) {
		g_object_weak_unref(G_OBJECT(window_info->window), window_destroyed, window_info);
	}

	g_free(window_info);
	return;
}

/**
 * hud_application_source_add_window:
 * @app: A #HudApplicationSource object
 * @window: The window to be added to the application
 *
 * Add a window to an application object.  Basically this means we only have to
 * have one BAMF listener in the application list.
 */
void
hud_application_source_add_window (HudApplicationSource * app, BamfWindow * window)
{
	g_return_if_fail(HUD_IS_APPLICATION_SOURCE(app));
	g_return_if_fail(BAMF_IS_WINDOW(window));

	guint32 xid = bamf_window_get_xid(window);

	if (app->priv->bamf_app == NULL) {
		g_debug("No BAMF application object");
		return;
	}

	window_info_t * window_info = g_new0(window_info_t, 1);
	window_info->window = window;
	window_info->xid = xid;
	window_info->source = app;

	g_object_weak_ref(G_OBJECT(window), window_destroyed, window_info);

	HudSourceList * collector_list = g_hash_table_lookup(app->priv->windows, GINT_TO_POINTER(xid));
	if (collector_list == NULL) {
		collector_list = hud_source_list_new();
		g_hash_table_insert(app->priv->windows, GINT_TO_POINTER(xid), collector_list);
	}

	/* We're managing the lifecycle of the window info here as
	   that allows it to have some sort of destroy function */
	g_object_set_data_full(G_OBJECT(collector_list), "hud-application-source-window-info", window_info, free_window_info);

	HudMenuModelCollector * mm_collector = NULL;
	HudDbusmenuCollector * dm_collector = NULL;
	GSList * sources = hud_source_list_get_list(collector_list);
	GSList * source;
	for (source = sources; source != NULL; source = g_slist_next(source)) {
		if (HUD_IS_MENU_MODEL_COLLECTOR(source->data)) {
			mm_collector = HUD_MENU_MODEL_COLLECTOR(source->data);
		}
		if (HUD_IS_DBUSMENU_COLLECTOR(source->data)) {
			dm_collector = HUD_DBUSMENU_COLLECTOR(source->data);
		}
	}

	const gchar * desktop_file = bamf_application_get_desktop_file(app->priv->bamf_app);
	const gchar * icon = bamf_view_get_icon(BAMF_VIEW(window));

	if (mm_collector == NULL) {
		mm_collector = hud_menu_model_collector_new(desktop_file, icon, 0);

		if (mm_collector != NULL) {
			hud_menu_model_collector_add_window(mm_collector, window);
			hud_source_list_add(collector_list, HUD_SOURCE(mm_collector));
		}
	}

	if (dm_collector == NULL) {
		dm_collector = hud_dbusmenu_collector_new_for_window(window, desktop_file, icon);

		if (dm_collector != NULL) {
			hud_source_list_add(collector_list, HUD_SOURCE(dm_collector));
		}
	}

	return;
}

/**
 * hud_application_source_has_xid:
 * @app: A #HudApplicationSource object
 * @xid: XID to lookup
 *
 * Looks to see if we know about this XID.
 *
 * Return value: Whether we're tracking @xid.
 */
gboolean
hud_application_source_has_xid (HudApplicationSource * app, guint32 xid)
{
	g_return_val_if_fail(HUD_IS_APPLICATION_SOURCE(app), FALSE);

	return g_hash_table_lookup(app->priv->windows, GINT_TO_POINTER(xid)) != NULL;
}
