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

struct _HudApplicationSourcePrivate {
	int dummy;
};

#define HUD_APPLICATION_SOURCE_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), HUD_APPLICATION_SOURCE_TYPE, HudApplicationSourcePrivate))

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

	return;
}

/* Clean up references */
static void
hud_application_source_dispose (GObject *object)
{

	G_OBJECT_CLASS (hud_application_source_parent_class)->dispose (object);
	return;
}

/* Free memory */
static void
hud_application_source_finalize (GObject *object)
{

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

	return;
}

HudApplicationSource *
hud_application_source_new_for_app (BamfApplication * bapp)
{

	return NULL;
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
