/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Pete Woods <pete.woods@canonical.com>
 */

#include "window-info.h"
#include <gobject/gobject.h>

struct _HudWindowInfo
{
  GObject parent_instance;
  guint32 window_id;
  gchar *app_id;
  HudWindowInfoStage stage;
  DBusWindowStack *window_stack;
  gchar *desktop_file;
};

typedef GObjectClass HudWindowInfoClass;

static void hud_window_info_finalize (GObject *object);
static void hud_window_info_dispose (GObject *object);

G_DEFINE_TYPE(HudWindowInfo, hud_window_info, G_TYPE_OBJECT);

static void
hud_window_info_class_init(GObjectClass *klass) {
	klass->dispose = hud_window_info_dispose;
	klass->finalize = hud_window_info_finalize;
}

static void
hud_window_info_init (HudWindowInfo *self)
{
}

HudWindowInfo *
hud_window_info_new(DBusWindowStack *window_stack, guint window_id,
		const gchar *app_id, HudWindowInfoStage stage)
{
	HudWindowInfo *self =
			HUD_WINDOW_INFO(g_object_new(HUD_TYPE_WINDOW_INFO, NULL));
	self->window_id = window_id;
	self->app_id = g_strdup(app_id);
	self->stage = stage;
	self->window_stack = g_object_ref(window_stack);
	const gchar *xdg_data_dirs = g_getenv("XDG_DATA_DIRS");
	if (xdg_data_dirs != NULL && xdg_data_dirs[0] != '\0') {
		gchar** dirs = g_strsplit(xdg_data_dirs, ":", 0);
		gchar **i = dirs;
		while (*i != NULL) {
			gchar *desktop_file = g_strdup_printf("%s/%s.desktop", *i, app_id);
			if (g_file_test(desktop_file, G_FILE_TEST_EXISTS)) {
				self->desktop_file = desktop_file;
				break;
			}
			g_free(desktop_file);
			++i;
		}
		g_strfreev(dirs);
	}
	if (self->desktop_file == NULL) {
		self->desktop_file = g_strdup_printf(
				"/usr/share/applications/%s.desktop", app_id);
	}
	return self;
}

static void
hud_window_info_dispose (GObject *object)
{
	HudWindowInfo *self = HUD_WINDOW_INFO(object);
	g_object_unref(self->window_stack);
}

static void
hud_window_info_finalize (GObject *object)
{
	HudWindowInfo *self = HUD_WINDOW_INFO(object);
	g_free(self->app_id);
	g_free(self->desktop_file);
}

guint32
hud_window_info_get_window_id(HudWindowInfo *self) {
	return self->window_id;
}

const gchar *
hud_window_info_get_app_id(HudWindowInfo *self) {
	return self->app_id;
}

const gchar *
hud_window_info_get_desktop_file(HudWindowInfo *self) {
	return self->desktop_file;
}

gchar *
hud_window_info_get_utf8_prop(HudWindowInfo *self,
		const gchar *property_name) {
	const gchar *property_names[] = { property_name, NULL };
	gchar** values = hud_window_info_get_utf8_properties(self,
			((const gchar * const *) &property_names));
	if(values == NULL) {
		return NULL;
	}
	gchar *result = g_strdup(values[0]);
	g_strfreev(values);
	return result;
}

gchar **
hud_window_info_get_utf8_properties(HudWindowInfo *self,
		const gchar * const *property_names) {
	gchar **result = NULL;
	GError *error = NULL;
	if (!dbus_window_stack_call_get_window_properties_sync(self->window_stack,
			self->window_id, self->app_id, property_names, &result, NULL,
			&error)) {
		g_warning("%s", error->message);
		g_error_free(error);
		return NULL;
	}
	return result;
}
