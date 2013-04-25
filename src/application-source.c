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

#define G_LOG_DOMAIN "hudapplicationsource"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "application-source.h"
#include "application-source-context.h"
#include "hudsource.h"
#include "app-iface.h"
#include "hudmenumodelcollector.h"
#include "huddbusmenucollector.h"
#include "hudsourcelist.h"

struct _HudApplicationSourcePrivate {
	GDBusConnection * session;

	gchar * app_id;
	gchar * path;
	AppIfaceComCanonicalHudApplication * skel;

#ifdef HAVE_BAMF
	AbstractApplication * bamf_app;
#endif
#ifdef HAVE_HYBRIS
	gchar * desktop_file;
#endif

	guint32 focused_window;
	char * current_context;

	GPtrArray * contexts;
	GHashTable * connections;
};

typedef struct _connection_watcher_t connection_watcher_t;
struct _connection_watcher_t {
	guint watch;
	GList * ids;
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
static void source_list_applications          (HudSource *                 hud_source,
                                               HudTokenList *              search_string,
                                               void                      (*append_func) (const gchar *application_id, const gchar *application_icon, HudSourceItemType type, gpointer user_data),
                                               gpointer                    user_data);
static void source_activate_toolbar           (HudSource *                 hud_source,
                                               HudClientQueryToolbarItems  item,
                                               GVariant                   *platform_data);
static HudSource * source_get                 (HudSource *                 hud_source,
                                               const gchar *               application_id);
static const gchar * source_get_app_id        (HudSource *                 hud_source);
static const gchar * source_get_app_icon      (HudSource *                 hud_source);
static gboolean dbus_add_sources              (AppIfaceComCanonicalHudApplication * skel,
                                               GDBusMethodInvocation *     invocation,
                                               GVariant *                  actions,
                                               GVariant *                  descs,
                                               gpointer                    user_data);
static gboolean dbus_set_context              (AppIfaceComCanonicalHudApplication * skel,
                                               GDBusMethodInvocation *     invocation,
                                               guint                       window_id,
                                               const gchar *               context,
                                               gpointer                    user_data);
static GList * source_get_items               (HudSource *                 object);

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
	iface->list_applications = source_list_applications;
	iface->get = source_get;
	iface->get_items = source_get_items;
	iface->activate_toolbar = source_activate_toolbar;
	iface->get_app_id = source_get_app_id;
	iface->get_app_icon = source_get_app_icon;

	return;
}

/* Free the struct and unwatch the name */
static void
connection_watcher_free (gpointer data)
{
	connection_watcher_t * watcher = (connection_watcher_t *)data;

	g_bus_unwatch_name(watcher->watch);
	g_list_free(watcher->ids);

	g_free(watcher);
	return;
}

/* Instance Init */
static void
hud_application_source_init (HudApplicationSource *self)
{
	self->priv = HUD_APPLICATION_SOURCE_GET_PRIVATE(self);

	self->priv->contexts = g_ptr_array_new_with_free_func(g_object_unref);
	self->priv->connections = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, connection_watcher_free);
	self->priv->session = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, NULL);

	return;
}

/* Clean up references */
static void
hud_application_source_dispose (GObject *object)
{
	HudApplicationSource * self = HUD_APPLICATION_SOURCE(object);

	g_clear_pointer(&self->priv->contexts, g_ptr_array_unref);
	g_clear_pointer(&self->priv->connections, g_hash_table_unref);

	if (self->priv->skel != NULL) {
		g_dbus_interface_skeleton_unexport(G_DBUS_INTERFACE_SKELETON(self->priv->skel));
		g_clear_object(&self->priv->skel);
	}

#ifdef HAVE_BAMF
	g_clear_object(&self->priv->bamf_app);
#endif
	g_clear_object(&self->priv->session);

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
	g_clear_pointer(&self->priv->current_context, g_free);
#ifdef HAVE_HYBRIS
	g_clear_pointer(&self->priv->desktop_file, g_free);
#endif

	G_OBJECT_CLASS (hud_application_source_parent_class)->finalize (object);
	return;
}

/* Checks to see if a context is in use */
static gboolean
context_is_current (HudApplicationSource * self, HudApplicationSourceContext * context)
{
	/* See if this window is the one we're looking at */
	guint32 context_window = hud_application_source_context_get_window_id(context);
	if (context_window != self->priv->focused_window && context_window != 0) {
		return FALSE;
	}

	/* Check the context too */
	const gchar * context_context = hud_application_source_context_get_context_id(context);
	if (context_context != NULL && g_strcmp0(context_context, self->priv->current_context) != 0) {
		return FALSE;
	}

	return TRUE;
}

/* Gets called when the items in a window's sources changes */
static void
window_source_changed (HudSource * source, gpointer user_data)
{
	HudApplicationSource * self = HUD_APPLICATION_SOURCE(user_data);
	HudApplicationSourceContext * context = HUD_APPLICATION_SOURCE_CONTEXT(source);

	if (context_is_current(self, context)) {
		hud_source_changed(HUD_SOURCE(self));
	}

	return;
}

/* Source interface using this source */
static void
source_use (HudSource *hud_source)
{
	HudApplicationSource * app = HUD_APPLICATION_SOURCE(hud_source);

	int i;
	for (i = 0; i < app->priv->contexts->len; i++) {
		HudApplicationSourceContext * context = g_ptr_array_index(app->priv->contexts, i);
		if (context_is_current(app, context)) {
			hud_source_use(HUD_SOURCE(context));
		}
	}

	return;
}

/* Source interface unusing this source */
static void
source_unuse (HudSource *hud_source)
{
	HudApplicationSource * app = HUD_APPLICATION_SOURCE(hud_source);

	int i;
	for (i = 0; i < app->priv->contexts->len; i++) {
		HudApplicationSourceContext * context = g_ptr_array_index(app->priv->contexts, i);
		if (context_is_current(app, context)) {
			hud_source_unuse(HUD_SOURCE(context));
		}
	}

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

	int i;
	for (i = 0; i < app->priv->contexts->len; i++) {
		HudApplicationSourceContext * context = g_ptr_array_index(app->priv->contexts, i);
		if (context_is_current(app, context)) {
			hud_source_search(HUD_SOURCE(context), search_string, append_func, user_data);
		}
	}

	return;
}

static void
source_list_applications (HudSource *     hud_source,
                          HudTokenList *  search_string,
                          void           (*append_func) (const gchar *application_id, const gchar *application_icon, HudSourceItemType type, gpointer user_data),
                          gpointer        user_data)
{
	HudApplicationSource * app = HUD_APPLICATION_SOURCE(hud_source);

	int i;
	for (i = 0; i < app->priv->contexts->len; i++) {
		HudApplicationSourceContext * context = g_ptr_array_index(app->priv->contexts, i);
		if (context_is_current(app, context)) {
			hud_source_list_applications(HUD_SOURCE(context), search_string, append_func, user_data);
		}
	}

	return;
}

static HudSource *
source_get (HudSource *     hud_source,
            const gchar *   application_id)
{
	HudApplicationSource * app = HUD_APPLICATION_SOURCE(hud_source);

	if (g_strcmp0 (application_id, app->priv->app_id) == 0) {
		return hud_source;
	}
	
	return NULL;
}

static const gchar *
source_get_app_id (HudSource * hud_source)
{
	return hud_application_source_get_id(HUD_APPLICATION_SOURCE(hud_source));
}

static const gchar *
source_get_app_icon (HudSource * hud_source)
{
	return hud_application_source_get_app_icon(HUD_APPLICATION_SOURCE(hud_source));
}

static void
source_activate_toolbar (HudSource * hud_source, HudClientQueryToolbarItems item, GVariant *platform_data)
{
	HudApplicationSource * app = HUD_APPLICATION_SOURCE(hud_source);

	int i;
	for (i = 0; i < app->priv->contexts->len; i++) {
		HudApplicationSourceContext * context = g_ptr_array_index(app->priv->contexts, i);
		if (context_is_current(app, context)) {
			hud_source_activate_toolbar (HUD_SOURCE(context), item, platform_data);
		}
	}

	return;
}

/**
 * hud_application_source_new_for_app:
 * @bapp: A #BamfApplication object
 *
 * Build a new application object, but use a #BamfApplication to help
 * ourselves.
 *
 * Return value: New #HudApplicationSource object
 */
HudApplicationSource *
hud_application_source_new_for_app (AbstractApplication * bapp)
{
	gchar * id = hud_application_source_bamf_app_id(bapp);
	if (id == NULL) {
		return NULL;
	}

	HudApplicationSource * source = hud_application_source_new_for_id(id);
	g_free(id);

	const gchar * desktop_file = NULL;
#ifdef HAVE_BAMF
	source->priv->bamf_app = g_object_ref(bapp);
	desktop_file = bamf_application_get_desktop_file(bapp);
#endif
#ifdef HAVE_HYBRIS
	source->priv->desktop_file = g_strdup(ubuntu_ui_session_properties_get_desktop_file_hint(*bapp));
	desktop_file = source->priv->desktop_file;
#endif

	app_iface_com_canonical_hud_application_set_desktop_path(source->priv->skel, desktop_file);

	return source;
}

/**
 * hud_application_source_new_for_id:
 * @id: The application ID
 *
 * Creates a new application source that doesn't have any windows, but is
 * based on the ID.  You should really add windows to this after you
 * create it.
 *
 * Return value: New #HudApplicationSource object
 */
HudApplicationSource *
hud_application_source_new_for_id (const gchar * id)
{
	HudApplicationSource * source = g_object_new(HUD_TYPE_APPLICATION_SOURCE, NULL);

	source->priv->app_id = g_strdup(id);

	source->priv->skel = app_iface_com_canonical_hud_application_skeleton_new();

	gchar * app_id_clean = g_strdup(id);
	gchar * app_id_cleanp;
	for (app_id_cleanp = app_id_clean; app_id_cleanp[0] != '\0'; app_id_cleanp++) {
		if (!g_ascii_isalnum(app_id_cleanp[0])) {
			app_id_cleanp[0] = '_';
		}
	}
	source->priv->path = g_strdup_printf("/com/canonical/hud/applications/%s", app_id_clean);

	int i = 0;
	GError * error = NULL;
	while (!g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(source->priv->skel),
	                                 source->priv->session,
	                                 source->priv->path,
	                                 &error)) {
		if (error != NULL) {
			g_warning("Unable to export application '%s' skeleton on path '%s': %s", id, source->priv->path, error->message);

			gboolean exists_error = g_error_matches(error, G_IO_ERROR, G_IO_ERROR_EXISTS);

			g_error_free(error);
			error = NULL;

			if (!exists_error) {
				break;
			}
		}
		g_free(source->priv->path);
		g_clear_object(&source->priv->skel);

		source->priv->path = g_strdup_printf("/com/canonical/hud/applications/%s_%d", app_id_clean, ++i);
		source->priv->skel = app_iface_com_canonical_hud_application_skeleton_new();
	}

	g_signal_connect(G_OBJECT(source->priv->skel), "handle-add-sources", G_CALLBACK(dbus_add_sources), source);
	g_signal_connect(G_OBJECT(source->priv->skel), "handle-set-window-context", G_CALLBACK(dbus_set_context), source);

	g_debug("Application ('%s') path: %s", id, source->priv->path);
	g_free(app_id_clean);

	return source;
}

/* Handle a name disappearing off of DBus */
static void
connection_lost (GDBusConnection * session, const gchar * name, gpointer user_data)
{
	HudApplicationSource * app = HUD_APPLICATION_SOURCE(user_data);

	connection_watcher_t * watcher = g_hash_table_lookup(app->priv->connections, name);
	if (watcher == NULL) {
		return;
	}

	gboolean focused_changed = FALSE;
	GList * idtemp;
	for (idtemp = watcher->ids; idtemp != NULL; idtemp = g_list_next(idtemp)) {
		guint32 winid = GPOINTER_TO_UINT(idtemp->data);

		if (winid == 0) {
			focused_changed = TRUE;
		}

		if (winid == app->priv->focused_window) {
			focused_changed = TRUE;
		}

		int i;
		for (i = 0; i < app->priv->contexts->len; i++) {
			HudApplicationSourceContext * context = g_ptr_array_index(app->priv->contexts, i);
			if (hud_application_source_context_get_window_id(context) == winid) {
				g_ptr_array_remove_index(app->priv->contexts, i);
				i--;
			}
		}
	}

	g_hash_table_remove(app->priv->connections, name);

	if (focused_changed) {
		/* If the focused window changed, let's tell everyone about it */
		hud_source_changed(HUD_SOURCE(app));
	} else if (app->priv->contexts->len == 0) {
		/* If we've not gotten to the situation of no more contexts, let's tell
		   our parent so they can reap us. */
		hud_source_changed(HUD_SOURCE(app));
	}

	return;
}

/* Adds to make sure we're tracking the ID for this DBus
   connection.  That way when it goes away, we know how to
   clean everything up. */
static void
add_id_to_connection (HudApplicationSource * app, GDBusConnection * session, const gchar * sender, guint32 id)
{
	connection_watcher_t * watcher = g_hash_table_lookup(app->priv->connections, sender);
	if (watcher == NULL) {
		watcher = g_new0(connection_watcher_t, 1);
		g_object_ref(app);
		watcher->watch = g_bus_watch_name_on_connection(session, sender, G_BUS_NAME_WATCHER_FLAGS_NONE, NULL, connection_lost, app, NULL);
		g_hash_table_insert(app->priv->connections, g_strdup(sender), watcher);
	}

	GList * idtemp;
	for (idtemp = watcher->ids; idtemp != NULL; idtemp = g_list_next(idtemp)) {
		guint32 listid = GPOINTER_TO_UINT(idtemp->data);

		if (listid == id) {
			return;
		}
	}

	watcher->ids = g_list_prepend(watcher->ids, GUINT_TO_POINTER(id));

	return;
}

/* Either find the context or build one */
HudApplicationSourceContext *
find_context (HudApplicationSource * app, GPtrArray * contexts, guint32 winid, const gchar * conid)
{
	HudApplicationSourceContext * retval = NULL;

	int i;
	for (i = 0; i < contexts->len; i++) {
		HudApplicationSourceContext * context = g_ptr_array_index(contexts, i);

		guint32 ctx_winid = hud_application_source_context_get_window_id(context);
		if (ctx_winid != winid) {
			continue;
		}

		const gchar * ctx_conid = hud_application_source_context_get_context_id(context);
		if (g_strcmp0(conid, ctx_conid) != 0) {
			continue;
		}

		retval = context;
		break;
	}

	/* Can't find, must build */
	if (retval == NULL) {
		retval = hud_application_source_context_new(winid, conid, app->priv->app_id, hud_application_source_get_app_icon(app), app->priv->path);
		g_ptr_array_add(contexts, retval);
	}

	return retval;
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

	guint32 idn = 0;
	gchar * context = NULL;
	gchar * prefix = NULL;
	gchar * object = NULL;

	/* NOTE: We are doing actions first as there are cases where
	   the models need the actions, but it'd be hard to update them
	   if we add the actions second.  This order is the best.  Don't
	   change it. */
	while (g_variant_iter_loop(&action_iter, "(usso)", &idn, &context, &prefix, &object)) {
		g_debug("Adding prefix '%s' at path: %s", prefix, object);

#ifdef HAVE_HYBRIS
		idn = WINDOW_ID_CONSTANT;
#endif

		HudApplicationSourceContext * ctx = find_context(app, app->priv->contexts, idn, NULL);

		GDBusActionGroup * ag = g_dbus_action_group_get(session, sender, object);
		hud_application_source_context_add_action_group(ctx, G_ACTION_GROUP(ag), prefix);

		add_id_to_connection(app, session, sender, idn);

		g_object_unref(ag);
	}

	GVariantIter desc_iter;
	g_variant_iter_init(&desc_iter, descs);

	while (g_variant_iter_loop(&desc_iter, "(uso)", &idn, &context, &object)) {
		g_debug("Adding descriptions: %s", object);

#ifdef HAVE_HYBRIS
		idn = WINDOW_ID_CONSTANT;
#endif

		HudApplicationSourceContext * ctx = find_context(app, app->priv->contexts, idn, NULL);

		GDBusMenuModel * model = g_dbus_menu_model_get(session, sender, object);
		hud_application_source_context_add_model(ctx, G_MENU_MODEL(model));

		add_id_to_connection(app, session, sender, idn);

		g_object_unref(model);
	}

	g_dbus_method_invocation_return_value(invocation, NULL);
	return TRUE;
}

/* Application changing the context for a window */
static gboolean
dbus_set_context (AppIfaceComCanonicalHudApplication * skel, GDBusMethodInvocation * invocation, guint window_id, const gchar * context, gpointer user_data)
{
	/* TODO: Use the data for something useful */

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

	return (app->priv->contexts->len == 0);
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
hud_application_source_bamf_app_id (AbstractApplication * bapp)
{
#ifdef HAVE_BAMF
	g_return_val_if_fail(BAMF_IS_APPLICATION(bapp), NULL);
#endif
#ifdef HAVE_HYBRIS
	/* Hybris has no way to check if the pointer is valid */
#endif

	const gchar * desktop_file = NULL;

#ifdef HAVE_BAMF
	desktop_file = bamf_application_get_desktop_file(bapp);
#endif
#ifdef HAVE_HYBRIS
	desktop_file = ubuntu_ui_session_properties_get_desktop_file_hint(*bapp);
#endif
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
hud_application_source_focus (HudApplicationSource * app, AbstractApplication * bapp, AbstractWindow * window)
{
	g_return_if_fail(HUD_IS_APPLICATION_SOURCE(app));
#ifdef HAVE_BAMF
	g_return_if_fail(BAMF_IS_APPLICATION(bapp));
#endif
#ifdef HAVE_HYBRIS
	/* Hybris has no way to check if the pointer is valid */
#endif

#ifdef HAVE_BAMF
	if (app->priv->bamf_app == NULL) {
		app->priv->bamf_app = g_object_ref(bapp);
	}

	g_return_if_fail(app->priv->bamf_app == bapp);
#endif
#ifdef HAVE_HYBRIS
	if (app->priv->desktop_file == NULL) {
		app->priv->desktop_file = g_strdup(ubuntu_ui_session_properties_get_desktop_file_hint(*bapp));
		app_iface_com_canonical_hud_application_set_desktop_path(app->priv->skel, app->priv->desktop_file);
	}
#endif

	hud_application_source_add_window(app, window);

#ifdef HAVE_BAMF
	app->priv->focused_window = bamf_window_get_xid(window);
#endif
#ifdef HAVE_HYBRIS
	app->priv->focused_window = _ubuntu_ui_session_properties_get_window_id(window);
#endif

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

/**
 * hud_application_source_get_app_icon:
 * @app: A #HudApplicationSource object
 *
 * Get the application icon
 *
 * Return value: The icon as a string
 */
const gchar *
hud_application_source_get_app_icon (HudApplicationSource * app)
{
	g_return_val_if_fail(HUD_IS_APPLICATION_SOURCE(app), NULL);

	const gchar * icon = NULL;
	const gchar * desktop_file = NULL;
#ifdef HAVE_BAMF
	desktop_file = bamf_application_get_desktop_file(app->priv->bamf_app);
#endif
#ifdef HAVE_HYBRIS
	desktop_file = app->priv->desktop_file;
#endif
	if (desktop_file != NULL) {
		GKeyFile * kfile = g_key_file_new();
		g_key_file_load_from_file(kfile, desktop_file, G_KEY_FILE_NONE, NULL);
		icon = g_key_file_get_value(kfile, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_ICON, NULL);
		g_key_file_free(kfile);
	}

	return icon;
}

typedef struct _window_info_t window_info_t;
struct _window_info_t {
	HudApplicationSource * source;  /* Not a ref */
#ifdef HAVE_BAMF
	AbstractWindow * window;        /* Not a ref */
#endif
	guint32 xid;                    /* Can't be a ref */
};

#ifdef HAVE_BAMF
/* When I window gets destroyed we want to clean up it's collectors
   and all that jazz. */
static void
window_destroyed (gpointer data, GObject * old_address)
{
	window_info_t * window_info = (window_info_t *)data;

	window_info->window = NULL;

	int i;
	for (i = 0; i < window_info->source->priv->contexts->len; i++) {
		HudApplicationSourceContext * context = g_ptr_array_index(window_info->source->priv->contexts, i);

		guint32 ctx_winid = hud_application_source_context_get_window_id(context);
		if (ctx_winid != window_info->xid) {
			continue;
		}

		g_ptr_array_remove_index(window_info->source->priv->contexts, i);
		i--;
	}

	if (window_info->xid == window_info->source->priv->focused_window) {
		hud_source_changed(HUD_SOURCE(window_info->source));
	}

	/* NOTE: DO NOT use the window_info after this point as
	   it may be free'd by the remove above. */

	return;
}
#endif

/* If the collector gets free'd first we need to deallocate the memory
   and make sure we don't keep the weak reference. */
static void
free_window_info (gpointer data)
{
	window_info_t * window_info = (window_info_t *)data;

#ifdef HAVE_BAMF
	if (window_info->window != NULL) {
		g_object_weak_unref(G_OBJECT(window_info->window), window_destroyed, window_info);
	}
#endif

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
hud_application_source_add_window (HudApplicationSource * app, AbstractWindow * window)
{
	g_return_if_fail(HUD_IS_APPLICATION_SOURCE(app));
#ifdef HAVE_BAMF
	g_return_if_fail(BAMF_IS_WINDOW(window));
#endif
#ifdef HAVE_HYBRIS
	/* Hybris has no way to check if the pointer is valid */
#endif

	guint32 xid = 0;
#ifdef HAVE_BAMF
	xid = bamf_window_get_xid(window);
#endif
#ifdef HAVE_HYBRIS
	xid = _ubuntu_ui_session_properties_get_window_id(window);
#endif

#ifdef HAVE_BAMF
	if (app->priv->bamf_app == NULL) {
		g_debug("No BAMF application object");
		return;
	}
#endif

	window_info_t * window_info = g_new0(window_info_t, 1);
	window_info->xid = xid;
	window_info->source = app;

#ifdef HAVE_BAMF
	/* Uhm, this is how we were managing this memory... uhg, hybris */
	window_info->window = window;
	g_object_weak_ref(G_OBJECT(window), window_destroyed, window_info);
#endif

	HudApplicationSourceContext * context = find_context(app, app->priv->contexts, xid, NULL);

	/* We're managing the lifecycle of the window info here as
	   that allows it to have some sort of destroy function */
	g_object_set_data_full(G_OBJECT(context), "hud-application-source-window-info", window_info, free_window_info);

#ifdef HAVE_BAMF
	gchar * app_id = hud_application_source_bamf_app_id(app->priv->bamf_app);
#endif
#ifdef HAVE_HYBRIS
	gchar * app_id = g_strdup(app->priv->app_id);
#endif
	const gchar * icon = NULL;
#ifdef HAVE_BAMF
	icon = bamf_view_get_icon(BAMF_VIEW(window));
#endif
#ifdef HAVE_BAMF
	/* Hybris can't find window icons, so we want to pull it from the desktop file */
#endif
	if (icon == NULL) {
		icon = hud_application_source_get_app_icon(app);
	}

	if (icon != NULL) {
		app_iface_com_canonical_hud_application_set_icon(app->priv->skel, icon);
	}

	hud_application_source_context_add_window(context, window);

	g_free (app_id);

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

	int i;
	for (i = 0; i < app->priv->contexts->len; i++) {
		HudApplicationSourceContext * context = g_ptr_array_index(app->priv->contexts, i);

		guint32 ctx_winid = hud_application_source_context_get_window_id(context);
		if (ctx_winid != xid) {
			return TRUE;
		}
	}

	return FALSE;
}

/* Gets all the items for the sources */
static GList *
source_get_items (HudSource * object)
{
	g_return_val_if_fail(HUD_IS_APPLICATION_SOURCE(object), NULL);
	HudApplicationSource *app = HUD_APPLICATION_SOURCE(object);
	GList * retval = NULL;

	int i;
	for (i = 0; i < app->priv->contexts->len; i++) {
		HudApplicationSourceContext * context = g_ptr_array_index(app->priv->contexts, i);
		if (context_is_current(app, context)) {
			retval = g_list_concat(hud_source_get_items(HUD_SOURCE(context)), retval);
		}
	}

	return retval;
}
