#ifndef __HUD_WINDOW_INFO_H__
#define __HUD_WINDOW_INFO_H__

#include <glib-object.h>
#include <window-stack-iface.h>

/*
 * Type macros.
 */
#define HUD_TYPE_WINDOW_INFO                  (hud_window_info_get_type ())
#define HUD_WINDOW_INFO(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), HUD_TYPE_WINDOW_INFO, HudWindowInfo))
#define HUD_IS_WINDOW_INFO(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HUD_TYPE_WINDOW_INFO))
#define HUD_WINDOW_INFO_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), HUD_TYPE_WINDOW_INFO, HudWindowInfoClass))
#define HUD_IS_WINDOW_INFO_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), HUD_TYPE_WINDOW_INFO))
#define HUD_WINDOW_INFO_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), HUD_TYPE_WINDOW_INFO, HudWindowInfoClass))

typedef struct _HudWindowInfo        HudWindowInfo;

#define WINDOW_ID_ALL_WINDOWS (0)

typedef enum {
  HUD_WINDOW_INFO_STAGE_MAIN,
  HUD_WINDOW_INFO_STAGE_SIDE,
  HUD_WINDOW_INFO_STAGE_WINDOWED,
} HudWindowInfoStage;

typedef HudWindowInfo HudApplicationInfo;

GType hud_window_info_get_type (void);

/*
 * Method definitions.
 */

HudWindowInfo * hud_window_info_new(DBusWindowStack *window_stack,
		guint window_id, const gchar *app_id, guint stack);

guint32 hud_window_info_get_window_id(HudWindowInfo *self);

const gchar * hud_window_info_get_app_id(HudWindowInfo *self);

const gchar * hud_window_info_get_desktop_file(HudWindowInfo *self);

gchar * hud_window_info_get_utf8_prop(HudWindowInfo *self,
		const gchar *property_name);

#endif /* __HUD_WINDOW_INFO_H__ */
