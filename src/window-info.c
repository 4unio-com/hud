
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

G_DEFINE_TYPE(HudWindowInfo, hud_window_info, G_TYPE_OBJECT);

static void
hud_window_info_class_init(GObjectClass *klass) {
	klass->finalize = hud_window_info_finalize;
}

static void
hud_window_info_init (HudWindowInfo *self)
{
}

HudWindowInfo *
hud_window_info_new(DBusWindowStack *window_stack, guint window_id,
		const gchar *app_id, HudWindowInfoStage stage) {
	HudWindowInfo *self =
			HUD_WINDOW_INFO(g_object_new(HUD_TYPE_WINDOW_INFO, NULL));
	self->window_id = window_id;
	self->app_id = g_strdup(app_id);
	self->stage = stage;
	self->window_stack = g_object_ref(window_stack);
	self->desktop_file = g_strdup_printf("/usr/share/applications/%s.desktop", app_id);
	return self;
}

static void
hud_window_info_finalize (GObject *object)
{
	HudWindowInfo *self = HUD_WINDOW_INFO(object);
	g_free(self->app_id);
	g_object_unref(self->window_stack);
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
hud_window_info_get_utf8_prop(HudWindowInfo *self, const gchar *property_name) {
	gchar *result = NULL;
	GError *error = NULL;
	if (!dbus_window_stack_call_get_window_property_sync(self->window_stack,
			self->window_id, self->app_id, property_name, &result, NULL,
			&error)) {
		g_warning("%s", error->message);
		g_error_free(error);
		return NULL;
	}
	if (result[0] == '\0') {
		g_free(result);
		result = NULL;
	}
	return result;
}
