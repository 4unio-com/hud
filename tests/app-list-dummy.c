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
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app-list-dummy.h"

typedef struct _AppListDummyPrivate AppListDummyPrivate;

struct _AppListDummyPrivate {
	HudSource * focused_source;
};

#define APP_LIST_DUMMY_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), APP_LIST_DUMMY_TYPE, AppListDummyPrivate))

static void app_list_dummy_class_init (AppListDummyClass *klass);
static void app_list_dummy_init       (AppListDummy *self);
static void app_list_dummy_dispose    (GObject *object);
static void app_list_dummy_finalize   (GObject *object);
static HudSource * get_focused_app (HudApplicationList * list);
static void matching_setup (HudApplicationList * list);

G_DEFINE_TYPE (AppListDummy, app_list_dummy, HUD_TYPE_APPLICATION_LIST);

static void
app_list_dummy_class_init (AppListDummyClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (AppListDummyPrivate));

	object_class->dispose = app_list_dummy_dispose;
	object_class->finalize = app_list_dummy_finalize;

	HudApplicationListClass * appclass = HUD_APPLICATION_LIST_CLASS(klass);
	appclass->get_focused_app = get_focused_app;

	appclass->matching_setup = matching_setup;

	return;
}

static void
app_list_dummy_init (AppListDummy *self)
{
	return;
}

static void
app_list_dummy_dispose (GObject *object)
{
	AppListDummyPrivate * priv = APP_LIST_DUMMY_GET_PRIVATE(object);

	g_clear_object(&priv->focused_source);

	G_OBJECT_CLASS (app_list_dummy_parent_class)->dispose (object);
	return;
}

static void
app_list_dummy_finalize (GObject *object)
{

	G_OBJECT_CLASS (app_list_dummy_parent_class)->finalize (object);
	return;
}

AppListDummy *
app_list_dummy_new (HudSource * focused_source)
{
	g_return_val_if_fail(HUD_IS_SOURCE(focused_source), NULL);

	AppListDummy * dummy = g_object_new(APP_LIST_DUMMY_TYPE, NULL);
	AppListDummyPrivate * priv = APP_LIST_DUMMY_GET_PRIVATE(dummy);

	priv->focused_source = g_object_ref(focused_source);
	return dummy;
}

static HudSource *
get_focused_app (HudApplicationList * list)
{
	g_return_val_if_fail(IS_APP_LIST_DUMMY(list), NULL);

	AppListDummyPrivate * priv = APP_LIST_DUMMY_GET_PRIVATE(list);

	return priv->focused_source;
}

static void
matching_setup (HudApplicationList * list)
{
	/* Wow, this is super-performant! */
	g_debug("Setting up matching");
	return;
}

void
app_list_dummy_set_focus (AppListDummy * dummy, HudSource * focused_source)
{
	g_return_if_fail(IS_APP_LIST_DUMMY(dummy));

	AppListDummyPrivate * priv = APP_LIST_DUMMY_GET_PRIVATE(dummy);
	g_clear_object(&priv->focused_source);
	priv->focused_source = g_object_ref(focused_source);

	hud_source_changed(HUD_SOURCE(dummy));

	return;
}
