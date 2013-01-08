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

/* Respond to the DBus function to add sources */
static gboolean
dbus_add_sources (AppIfaceComCanonicalHudApplication * skel, GDBusMethodInvocation * invocation, GVariant * actions, GVariant * descs, gpointer user_data)
{
	GVariantIter action_iter;
	g_variant_iter_init(&action_iter, actions);

	GVariant * id = NULL;
	gchar * prefix = NULL;
	gchar * object = NULL;

	while (g_variant_iter_loop(&action_iter, "(vso)", id, prefix, object)) {
		g_debug("Adding prefix '%s' at path: %s", prefix, object);
	}

	GVariantIter desc_iter;
	g_variant_iter_init(&desc_iter, descs);

	while (g_variant_iter_loop(&desc_iter, "(vo)", id, object)) {
		g_debug("Adding descriptions: %s", object);
	}

	g_dbus_method_invocation_return_value(invocation, NULL);
	return TRUE;
}

gboolean
hud_application_source_is_empty (HudApplicationSource * app)
{

	return TRUE;
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

	HudSource * collector = g_hash_table_lookup(app->priv->windows, GINT_TO_POINTER(xid));
	if (collector != NULL) {
		g_debug("Got it");
		return;
	}

	if (app->priv->bamf_app == NULL) {
		g_debug("No BAMF application object");
		return;
	}

	/* TODO: This is from other code that states this, the assumption
	   is incorrect.  */
	/* GMenuModel menus either exist at the start or will never exist.
	 * dbusmenu menus can appear later.
	 *
	 * For that reason, we check first for GMenuModel and assume if it
	 * doesn't exist then it must be dbusmenu.
	 */

	const gchar * desktop_file = bamf_application_get_desktop_file(app->priv->bamf_app);
	const gchar * icon = bamf_view_get_icon(BAMF_VIEW(window));

	HudMenuModelCollector * menumodel_collector = NULL;
	menumodel_collector = hud_menu_model_collector_get(window, desktop_file, icon);

	if (menumodel_collector != NULL) {
		collector = HUD_SOURCE(menumodel_collector);
	} else {
		collector = HUD_SOURCE(hud_dbusmenu_collector_new_for_window(window, desktop_file, icon));
	}

	g_hash_table_insert(app->priv->windows, GINT_TO_POINTER(xid), collector);

	return;
}
