#ifndef __HUD_WATCHDOG_H__
#define __HUD_WATCHDOG_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define HUD_WATCHDOG_TYPE            (hud_watchdog_get_type ())
#define HUD_WATCHDOG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), HUD_WATCHDOG_TYPE, HudWatchdog))
#define HUD_WATCHDOG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), HUD_WATCHDOG_TYPE, HudWatchdogClass))
#define IS_HUD_WATCHDOG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HUD_WATCHDOG_TYPE))
#define IS_HUD_WATCHDOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), HUD_WATCHDOG_TYPE))
#define HUD_WATCHDOG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), HUD_WATCHDOG_TYPE, HudWatchdogClass))

typedef struct _HudWatchdog      HudWatchdog;
typedef struct _HudWatchdogClass HudWatchdogClass;

struct _HudWatchdogClass {
	GObjectClass parent_class;
};

struct _HudWatchdog {
	GObject parent;
};

GType hud_watchdog_get_type (void);

HudWatchdog * hud_watchdog_new  (GMainLoop * loop);
void          hud_watchdog_ping (HudWatchdog * watchdog);

G_END_DECLS

#endif
