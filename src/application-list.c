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

#define G_LOG_DOMAIN "hudapplicationlist"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "window-info.h"

#include "application-list.h"
#include "application-source.h"
#include "source.h"
#include "window-stack-iface.h"

typedef struct _HudApplicationListPrivate HudApplicationListPrivate;

struct _HudApplicationListPrivate {
	HudApplicationSource * last_focused_main_stage_source;

	DBusWindowStack * window_stack;
	gulong matcher_app_sig;
	gulong matcher_view_open_sig;
	gulong matcher_view_close_sig;
	GHashTable * applications;
	HudSource * used_source;
};

#define HUD_APPLICATION_LIST_GET_PRIVATE(o) \
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), HUD_TYPE_APPLICATION_LIST, HudApplicationListPrivate))

static void hud_application_list_class_init (HudApplicationListClass * klass);
static void hud_application_list_init       (HudApplicationList *      self);
static void hud_application_list_constructed (GObject * object);
static void matching_setup_bamf             (HudApplicationList *      self);
static void hud_application_list_dispose    (GObject *                 object);
static void hud_application_list_finalize   (GObject *                 object);
static void source_iface_init               (HudSourceInterface *      iface);
static void window_changed                  (DBusWindowStack *         matcher,
                                             guint                     window_id,
                                             const gchar *             app_id,
                                             guint                     stage,
                                             gpointer                  user_data);
static void view_opened                     (DBusWindowStack *         matcher,
                                             guint                     window_id,
                                             const gchar *             app_id,
                                             gpointer                  user_data);
static void view_closed                     (DBusWindowStack *         matcher,
                                             guint                     window_id,
                                             const gchar *             app_id,
                                             gpointer                  user_data);
static void source_use                      (HudSource *               hud_source);
static void source_unuse                    (HudSource *               hud_source);
static void source_search                   (HudSource *               hud_source,
                                             HudTokenList *            search_string,
                                             void                    (*append_func) (HudResult * result, gpointer user_data),
                                             gpointer                  user_data);
static void source_list_applications        (HudSource *               hud_source,
                                             HudTokenList *            search_string,
                                             void                    (*append_func) (const gchar *application_id, const gchar *application_icon, HudSourceItemType type, gpointer user_data),
                                             gpointer                  user_data);
static HudSource * source_get               (HudSource *               hud_source,
                                             const gchar *             application_id);
static GList * source_get_items             (HudSource *               list);
static void application_source_changed      (HudSource *               source,
                                             gpointer                  user_data);
static gboolean hud_application_list_name_in_ignore_list (HudWindowInfo *window);

G_DEFINE_TYPE_WITH_CODE (HudApplicationList, hud_application_list, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (HUD_TYPE_SOURCE, source_iface_init))

/* Class Init */
static void
hud_application_list_class_init (HudApplicationListClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (HudApplicationListPrivate));

	object_class->constructed = hud_application_list_constructed;
	object_class->dispose = hud_application_list_dispose;
	object_class->finalize = hud_application_list_finalize;

	klass->matching_setup = matching_setup_bamf;

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

	return;
}

/* Instance Init */
static void
hud_application_list_init (HudApplicationList *self)
{
	self->priv = HUD_APPLICATION_LIST_GET_PRIVATE(self);
	self->priv->applications = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_object_unref);

	return;
}

/* Final build steps */
static void
hud_application_list_constructed (GObject * object)
{
	HudApplicationList * self = HUD_APPLICATION_LIST(object);

	HudApplicationListClass * aclass = HUD_APPLICATION_LIST_GET_CLASS(self);
	if (aclass->matching_setup != NULL) {
		aclass->matching_setup(self);
	}

	return;
}

static void
matching_setup_bamf (HudApplicationList * self)
{
	GError *error = NULL;
	self->priv->window_stack = dbus_window_stack_proxy_new_for_bus_sync(
			G_BUS_TYPE_SESSION, G_DBUS_PROXY_FLAGS_NONE,
			"com.canonical.Unity.WindowStack",
			"/com/canonical/Unity/WindowStack",
			NULL, &error);
	if(self->priv->window_stack == NULL) {
		g_warning("Could not construct window stack proxy: %s", error->message);
		g_error_free(error);
		return;
	}

	g_debug("connecting to window stack signals");
	self->priv->matcher_app_sig = g_signal_connect(self->priv->window_stack,
		"focused-window-changed",
		G_CALLBACK(window_changed), self);
	self->priv->matcher_view_open_sig = g_signal_connect(self->priv->window_stack,
		"window-created",
		G_CALLBACK(view_opened), self);
	self->priv->matcher_view_close_sig = g_signal_connect(self->priv->window_stack,
		"window-destroyed",
		G_CALLBACK(view_closed), self);
	g_debug("connected to window stack signals");

	GVariant *stack = NULL;
	error = NULL;
	if (!dbus_window_stack_call_get_window_stack_sync(self->priv->window_stack,
			&stack, NULL, &error)) {
		g_warning("Could not get window stack: %s", error->message);
		g_error_free(error);
		return;
	}

//	GList * apps = bamf_matcher_get_applications(self->priv->matcher);
//	GList * app = NULL;
//	for (app = apps; app != NULL; app = g_list_next(app)) {
//		if (!BAMF_IS_APPLICATION(app->data)) {
//			continue;
//		}
//
//		BamfApplication * bapp = BAMF_APPLICATION(app->data);
//		gchar * app_id = hud_application_source_bamf_app_id(bapp);
//
//		if (app_id == NULL) {
//			continue;
//		}
//
//		HudApplicationSource * appsource = hud_application_source_new_for_app(bapp);
//		g_signal_connect(appsource, "changed", G_CALLBACK(application_source_changed), self);
//
//		if (!hud_application_source_is_empty(appsource)) {
//			g_hash_table_insert(self->priv->applications, app_id, appsource);
//		} else {
//			g_object_unref(appsource);
//		}
//	}
//
//	GList * windows = bamf_matcher_get_windows(self->priv->matcher);
//	GList * window = NULL;
//	for (window = windows; window != NULL; window = g_list_next(window)) {
//		if (!BAMF_IS_WINDOW(window->data)) {
//			continue;
//		}
//
//		view_opened(self->priv->matcher, BAMF_VIEW(window->data), self);
//	}
//
//	BamfWindow * focused = bamf_matcher_get_active_window(self->priv->matcher);
//	if (focused != NULL && !hud_application_list_name_in_ignore_list(focused)) {
//		window_changed(self->priv->matcher, NULL, focused, self);
//	} else {
//		GList * stack = bamf_matcher_get_window_stack_for_monitor(self->priv->matcher, -1);
//
//		GList * last = g_list_last(stack);
//		while (last != NULL) {
//			if (hud_application_list_name_in_ignore_list(last->data)) {
//				last = g_list_previous(last);
//				continue;
//			}
//
//			window_changed(self->priv->matcher, NULL, last->data, self);
//			break;
//		}
//
//		g_list_free(stack);
//	}
}

/* Clean up references */
static void
hud_application_list_dispose (GObject *object)
{
	HudApplicationList * self = HUD_APPLICATION_LIST(object);
	g_debug("Application List Dispose Start");

	if (self->priv->used_source != NULL) {
		hud_source_unuse(self->priv->used_source);
		g_clear_object(&self->priv->used_source);
	}

	g_clear_object(&self->priv->last_focused_main_stage_source);

	if (self->priv->matcher_app_sig != 0 && self->priv->window_stack != NULL) {
		g_signal_handler_disconnect(self->priv->window_stack, self->priv->matcher_app_sig);
	}
	self->priv->matcher_app_sig = 0;

	if (self->priv->matcher_view_open_sig != 0 && self->priv->window_stack != NULL) {
		g_signal_handler_disconnect(self->priv->window_stack, self->priv->matcher_view_open_sig);
	}
	self->priv->matcher_view_open_sig = 0;

	if (self->priv->matcher_view_close_sig != 0 && self->priv->window_stack != NULL) {
		g_signal_handler_disconnect(self->priv->window_stack, self->priv->matcher_view_close_sig);
	}
	self->priv->matcher_view_close_sig = 0;

	g_debug("Unrefing window stack");
	g_clear_object(&self->priv->window_stack);
	g_debug("Unref'd window stack");

	g_hash_table_remove_all(self->priv->applications);

	g_debug("Application List Dispose Recurse");
	G_OBJECT_CLASS (hud_application_list_parent_class)->dispose (object);
	g_debug("Application List Dispose Stop");
	return;
}

/* Free memory */
static void
hud_application_list_finalize (GObject *object)
{
	HudApplicationList * self = HUD_APPLICATION_LIST(object);
	g_debug("Application List Finalize Start");

	g_clear_pointer(&self->priv->applications, g_hash_table_unref);

	g_debug("Application List Finalize Recurse");
	G_OBJECT_CLASS (hud_application_list_parent_class)->finalize (object);
	g_debug("Application List Finalize Stop");
	return;
}

/* Get a source from a BamfApp */
static HudApplicationSource *
bamf_app_to_source (HudApplicationList * list, HudApplicationInfo * bapp)
{
	const gchar * id = hud_window_info_get_app_id(bapp);

	HudApplicationSource * source = g_hash_table_lookup(list->priv->applications, id);
	if (source == NULL) {
		source = hud_application_source_new_for_app(bapp);
		g_signal_connect(source, "changed", G_CALLBACK(application_source_changed), list);

		g_hash_table_insert(list->priv->applications, g_strdup(id), source);
		id = NULL; /* We used the malloc in the table */

		hud_source_changed(HUD_SOURCE(list));
	}

	return source;
}

static gboolean
hud_application_list_name_in_ignore_list (HudWindowInfo *window)
{
  static const gchar * const ignored_names[] = {
    "Hud Prototype Test",
    "Hud",
    "DNDCollectionWindow",
    "launcher",
    "dash",
    "Dash",
    "panel",
    "hud",
    "unity-2d-shell",
    "unity-dash",
    "unity-panel",
    "unity-launcher",
    "XdndCollectionWindowImp",
  };
  gboolean ignored = FALSE;
  const gchar *window_name = NULL;
  gint i;

  //FIXME Can we get the name?
  window_name = hud_window_info_get_utf8_prop(window, "WM_NAME");
//  window_name = hud_window_info_get_app_id(window);
  g_debug ("checking window name '%s'", window_name);

  /* sometimes bamf returns NULL here... protect ourselves */
  if (window_name == NULL)
    return TRUE;

  for (i = 0; i < G_N_ELEMENTS (ignored_names); i++)
    if (g_str_equal (ignored_names[i], window_name))
      {
        g_debug ("window name '%s' blocked", window_name);
        ignored = TRUE;
        break;
      }

  return ignored;
}

/* Called each time the focused application changes */
static void
window_changed (DBusWindowStack *window_stack, guint window_id, const gchar *app_id, guint stack, gpointer user_data)
{
	HudApplicationList * list = HUD_APPLICATION_LIST(user_data);

	HudWindowInfo *window = hud_window_info_new(list->priv->window_stack, window_id, app_id, stack);

	if (hud_application_list_name_in_ignore_list (window))
	    return;

	/* Clear the last source, as we've obviously changed */
	g_clear_object(&list->priv->last_focused_main_stage_source);

	/* Try to use BAMF */
//	BamfApplication * new_app = bamf_matcher_get_application_for_window(list->priv->matcher, new_win);
//	if (new_app == NULL || bamf_application_get_desktop_file(new_app) == NULL) {
//		/* We can't handle things we can't identify */
//		hud_source_changed(HUD_SOURCE(list));
//		return;
//	}

	HudApplicationSource * source = NULL;

	/* If we've got an app, we can find it easily */
//	if (new_app != NULL) {
		source = bamf_app_to_source(list, window);
//	}

	/* If we weren't able to use BAMF, let's try to find a source
	   for the window. */
	if (source == NULL) {
		guint xid = hud_window_info_get_window_id(window);
		GList * sources = g_hash_table_get_values(list->priv->applications);
		GList * lsource = NULL;

		for (lsource = sources; lsource != NULL; lsource = g_list_next(lsource)) {
			HudApplicationSource * appsource = HUD_APPLICATION_SOURCE(lsource->data);
			if (appsource == NULL) continue;

			if (hud_application_source_has_xid(appsource, xid)) {
				source = appsource;
				break;
			}
		}
	}

	if (source == NULL) {
		g_warning("Unable to find source for window");
		return;
	}

	list->priv->last_focused_main_stage_source = g_object_ref(source);

	hud_application_source_focus(source, window, window);

	hud_source_changed(HUD_SOURCE(list));

	return;
}


/* A new view has been opened by BAMF */
static void
view_opened (DBusWindowStack * window_stack, guint window_id, const gchar *app_id, gpointer user_data)
{
	HudApplicationList * list = HUD_APPLICATION_LIST(user_data);

	HudWindowInfo *window = hud_window_info_new(list->priv->window_stack,
			window_id, app_id, HUD_WINDOW_INFO_STAGE_MAIN);

//	BamfApplication * app = bamf_matcher_get_application_for_window(list->priv->matcher, BAMF_WINDOW(view));
//	if (app == NULL) {
//		return;
//	}

	HudApplicationSource * source = bamf_app_to_source(list, window);
	if (source == NULL) {
		return;
	}

	hud_application_source_add_window(source, window);

	return;
}

/* A view has been closed by BAMF */
static void
view_closed (DBusWindowStack * window_stack, guint window_id, const gchar *app_id, gpointer user_data)
{
	g_debug("Closing Window: %d", window_id);

	HudApplicationList * list = HUD_APPLICATION_LIST(user_data);

	GList * sources = g_hash_table_get_values(list->priv->applications);
	GList * lsource = NULL;

	for (lsource = sources; lsource != NULL; lsource = g_list_next(lsource)) {
		HudApplicationSource * appsource = HUD_APPLICATION_SOURCE(lsource->data);
		if (appsource == NULL) continue;

		if (hud_application_source_has_xid(appsource, window_id)) {
			hud_application_source_window_closed(appsource, window_id);
		}
	}

	return;
}

/* Source interface using this source */
static void
source_use (HudSource *hud_source)
{
	g_return_if_fail(HUD_IS_APPLICATION_LIST(hud_source));
	HudApplicationList * list = HUD_APPLICATION_LIST(hud_source);

	HudApplicationSource * source = NULL;

	/* First see if we've already got it */
	source = list->priv->last_focused_main_stage_source;

//	if (source == NULL) {
//		/* Try using the application first */
//		HudApplicationInfo * app = NULL;
//		app = bamf_matcher_get_active_application(list->priv->matcher);
//
//		if (app != NULL) {
//			source = bamf_app_to_source(list, app);
//		}
//	}

	/* If we weren't able to use BAMF, let's try to find a source
	   for the window. */
//	if (source == NULL) {
//		guint32 xid = 0;
//
//		xid = bamf_window_get_xid(bamf_matcher_get_active_window(list->priv->matcher));
//
//		GList * sources = g_hash_table_get_values(list->priv->applications);
//		GList * lsource = NULL;
//
//		for (lsource = sources; lsource != NULL; lsource = g_list_next(lsource)) {
//			HudApplicationSource * appsource = HUD_APPLICATION_SOURCE(lsource->data);
//			if (appsource == NULL) continue;
//
//			if (hud_application_source_has_xid(appsource, xid)) {
//				source = appsource;
//				break;
//			}
//		}
//	}

	if (source == NULL) {
		g_warning("Unable to find source for window");
		return;
	}

	if (list->priv->used_source != NULL) {
		hud_source_unuse(list->priv->used_source);
		g_clear_object(&list->priv->used_source);
	}

	list->priv->used_source = g_object_ref(source);

	hud_source_use(HUD_SOURCE(source));

	return;
}

/* Source interface unusing this source */
static void
source_unuse (HudSource *hud_source)
{
	g_return_if_fail(HUD_IS_APPLICATION_LIST(hud_source));
	HudApplicationList * list = HUD_APPLICATION_LIST(hud_source);

	g_return_if_fail(list->priv->used_source != NULL);

	hud_source_unuse(list->priv->used_source);
	g_clear_object(&list->priv->used_source);

	return;
}

/* Search this source */
static void
source_search (HudSource *     hud_source,
               HudTokenList *  search_string,
               void          (*append_func) (HudResult * result, gpointer user_data),
               gpointer        user_data)
{
	g_return_if_fail(HUD_IS_APPLICATION_LIST(hud_source));
	HudApplicationList * list = HUD_APPLICATION_LIST(hud_source);

	g_return_if_fail(list->priv->used_source != NULL);

	hud_source_search(list->priv->used_source, search_string, append_func, user_data);

	return;
}

static void
source_list_applications (HudSource *               hud_source,
                          HudTokenList *            search_string,
                          void                    (*append_func) (const gchar *application_id, const gchar *application_icon, HudSourceItemType type, gpointer user_data),
                          gpointer                  user_data)
{
	g_return_if_fail(HUD_IS_APPLICATION_LIST(hud_source));
	HudApplicationList * list = HUD_APPLICATION_LIST(hud_source);
	GList * sources = g_hash_table_get_values(list->priv->applications);
	GList * lsource = NULL;

	for (lsource = sources; lsource != NULL; lsource = g_list_next(lsource)) {
		HudApplicationSource * appsource = HUD_APPLICATION_SOURCE(lsource->data);
		if (appsource == NULL || HUD_SOURCE(appsource) == list->priv->used_source) continue;

		hud_source_list_applications(HUD_SOURCE(appsource), search_string, append_func, user_data);
	}
	
	if (list->priv->used_source != NULL) {
		hud_source_list_applications(list->priv->used_source, search_string, append_func, user_data);
	}
}

static HudSource *
source_get (HudSource *     hud_source,
            const gchar *   application_id)
{
	g_return_val_if_fail(HUD_IS_APPLICATION_LIST(hud_source), NULL);
	g_return_val_if_fail(application_id != NULL, NULL);
	HudApplicationList * list = HUD_APPLICATION_LIST(hud_source);

	return g_hash_table_lookup(list->priv->applications, application_id);
}

/* An application has signaled that it's items have changed */
static void
application_source_changed (HudSource * source, gpointer user_data)
{
	HudApplicationList * list = HUD_APPLICATION_LIST(user_data);
	HudApplicationSource * appsource = HUD_APPLICATION_SOURCE(source);

	/* If the application source has become empty it means that it
	 * the correspongind app has terminated and it's time to do the
	 * cleanup.
	 */
	if (hud_application_source_is_empty (appsource)) {
		if ((gpointer)appsource == (gpointer)list->priv->used_source) {
			hud_source_unuse(HUD_SOURCE(appsource));
			g_clear_object(&list->priv->used_source);
		}

		gchar * id = g_strdup(hud_application_source_get_id(appsource));

		g_hash_table_remove(list->priv->applications, id);
		g_free(id);
	}

	hud_source_changed(HUD_SOURCE(list));

	return;
}

/**
 * hud_application_list_new:
 *
 * Create a new application list.
 *
 * Return Value: (transfer full): New #HudApplicationList
 */
HudApplicationList *
hud_application_list_new (void)
{
	return g_object_new(HUD_TYPE_APPLICATION_LIST,
	                    NULL);
}

/**
 * hud_application_list_get_source:
 * @list: A #HudApplicationList object
 * @id: Application ID to find
 *
 * Looks for a source in the application list database or if it
 * doesn't exist, it creates it.
 *
 * Return value: (transfer none): An #HudApplicationSource matching @id
 */
HudApplicationSource *
hud_application_list_get_source (HudApplicationList * list, const gchar * id)
{
	g_return_val_if_fail(HUD_IS_APPLICATION_LIST(list), NULL);
	g_return_val_if_fail(id != NULL, NULL);

	if (g_strcmp0(id, "gallery") == 0) {
		id = "goodhope";
	}

	HudApplicationSource * source = HUD_APPLICATION_SOURCE(source_get(HUD_SOURCE(list), id));
	if (source == NULL) {
		source = hud_application_source_new_for_id(id);
		g_signal_connect(source, "changed", G_CALLBACK(application_source_changed), list);
		g_hash_table_insert(list->priv->applications, g_strdup(id), source);
	}

	return source;
}

/**
 * hud_application_list_get_focused_app:
 * @list: A #HudApplicationList object
 * 
 * Gets the focused app source
 *
 * Return value: (transfer none): The current #HudApplicationSource
 */
HudSource *
hud_application_list_get_focused_app (HudApplicationList * list)
{
	g_return_val_if_fail(HUD_IS_APPLICATION_LIST(list), NULL);

	HudApplicationListClass * aclass = HUD_APPLICATION_LIST_GET_CLASS(list);
	if (G_UNLIKELY(aclass->get_focused_app != NULL)) {
		return aclass->get_focused_app(list);
	}

	return HUD_SOURCE(list->priv->last_focused_main_stage_source);
}

/**
 * hud_application_list_get_side_stage_focused_app:
 * @list: A #HudApplicationList object
 * 
 * Gets the side stage focused app source
 *
 * Return value: (transfer none): The current #HudApplicationSource
 */
HudSource *
hud_application_list_get_side_stage_focused_app (HudApplicationList * list)
{
    g_return_val_if_fail(HUD_IS_APPLICATION_LIST(list), NULL);

    return NULL;
    //FIXME: return something like HUD_SOURCE(list->priv->last_focused_side_stage_source);
}

/**
 * hud_application_list_get_apps:
 * @list: A #HudApplicationList object
 * 
 * Gets a list of applications
 *
 * Return value: A list of #HudApplicationSource objects
 */
GList *
hud_application_list_get_apps (HudApplicationList * list)
{
  g_return_val_if_fail(HUD_IS_APPLICATION_LIST(list), NULL);

  return g_hash_table_get_values(list->priv->applications);
}

/**
 * hud_application_list_get_active_collector:
 *
 * Returns the active collector if there is one
 *
 * Returns: (transfer none): A #HudCollector or NULL if none
 */
GList *
source_get_items (HudSource * source)
{
  g_return_val_if_fail(HUD_IS_APPLICATION_LIST(source), NULL);
  HudApplicationList *list = HUD_APPLICATION_LIST(source);
  g_return_val_if_fail(list->priv->used_source != NULL, NULL);
  return hud_source_get_items(list->priv->used_source);
}
