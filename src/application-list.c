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

#include <libbamf/libbamf.h>

#include "application-list.h"
#include "application-source.h"
#include "hudsource.h"

typedef struct _HudApplicationListPrivate HudApplicationListPrivate;

struct _HudApplicationListPrivate {
	BamfMatcher * matcher;
	gulong matcher_sig;

	GHashTable * applications;
};

#define HUD_APPLICATION_LIST_GET_PRIVATE(o) \
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), HUD_TYPE_APPLICATION_LIST, HudApplicationListPrivate))

static void hud_application_list_class_init (HudApplicationListClass * klass);
static void hud_application_list_init       (HudApplicationList *      self);
static void hud_application_list_dispose    (GObject *                 object);
static void hud_application_list_finalize   (GObject *                 object);
static void source_iface_init               (HudSourceInterface *      iface);
static void application_changed             (BamfMatcher *             matcher,
                                             BamfApplication *         old_app,
                                             BamfApplication *         new_app,
                                             gpointer                  user_data);
static void source_use                      (HudSource *               hud_source);
static void source_unuse                    (HudSource *               hud_source);
static void source_search                   (HudSource *               hud_source,
                                             HudTokenList *            search_string,
                                             void                    (*append_func) (HudResult * result, gpointer user_data),
                                             gpointer                  user_data);

G_DEFINE_TYPE_WITH_CODE (HudApplicationList, hud_application_list, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (HUD_TYPE_SOURCE, source_iface_init))

/* Class Init */
static void
hud_application_list_class_init (HudApplicationListClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (HudApplicationListPrivate));

	object_class->dispose = hud_application_list_dispose;
	object_class->finalize = hud_application_list_finalize;

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
hud_application_list_init (HudApplicationList *self)
{
	self->priv = HUD_APPLICATION_LIST_GET_PRIVATE(self);

	self->priv->matcher = bamf_matcher_get_default();
	self->priv->matcher_sig = g_signal_connect(self->priv->matcher,
		"active-application-changed",
		G_CALLBACK(application_changed), self);

	self->priv->applications = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_object_unref);

	GList * apps = bamf_matcher_get_applications(self->priv->matcher);
	GList * app = NULL;
	for (app = apps; app != NULL; app = g_list_next(app)) {
		if (!BAMF_IS_APPLICATION(app->data)) {
			continue;
		}

		BamfApplication * bapp = BAMF_APPLICATION(app->data);
		gchar * app_id = hud_application_source_bamf_app_id(bapp);

		if (app_id == NULL) {
			continue;
		}

		HudApplicationSource * appsource = hud_application_source_new_for_app(bapp);

		if (!hud_application_source_is_empty(appsource)) {
			g_hash_table_insert(self->priv->applications, app_id, appsource);
		} else {
			g_object_unref(appsource);
		}
	}
	g_list_free_full(apps, g_object_unref);

	return;
}

/* Clean up references */
static void
hud_application_list_dispose (GObject *object)
{
	HudApplicationList * self = HUD_APPLICATION_LIST(object);

	if (self->priv->matcher_sig != 0 && self->priv->matcher != NULL) {
		g_signal_handler_disconnect(self->priv->matcher, self->priv->matcher_sig);
	}
	self->priv->matcher_sig = 0;
	g_clear_object(&self->priv->matcher);

	g_clear_pointer(&self->priv->applications, g_hash_table_unref);

	G_OBJECT_CLASS (hud_application_list_parent_class)->dispose (object);
	return;
}

/* Free memory */
static void
hud_application_list_finalize (GObject *object)
{

	G_OBJECT_CLASS (hud_application_list_parent_class)->finalize (object);
	return;
}

/* Called each time the focused application changes */
static void
application_changed (BamfMatcher * matcher, BamfApplication * old_app, BamfApplication * new_app, gpointer user_data)
{


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
