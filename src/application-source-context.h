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
 *
 * Author: Ted Gould <ted@canonical.com>
 */

#ifndef __HUD_APPLICATION_SOURCE_CONTEXT_H__
#define __HUD_APPLICATION_SOURCE_CONTEXT_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define HUD_TYPE_APPLICATION_SOURCE_CONTEXT            (hud_application_source_context_get_type ())
#define HUD_APPLICATION_SOURCE_CONTEXT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), HUD_TYPE_APPLICATION_SOURCE_CONTEXT, HudApplicationSourceContext))
#define HUD_APPLICATION_SOURCE_CONTEXT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), HUD_TYPE_APPLICATION_SOURCE_CONTEXT, HudApplicationSourceContextClass))
#define HUD_IS_APPLICATION_SOURCE_CONTEXT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HUD_TYPE_APPLICATION_SOURCE_CONTEXT))
#define HUD_IS_APPLICATION_SOURCE_CONTEXT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), HUD_TYPE_APPLICATION_SOURCE_CONTEXT))
#define HUD_APPLICATION_SOURCE_CONTEXT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), HUD_TYPE_APPLICATION_SOURCE_CONTEXT, HudApplicationSourceContextClass))

typedef struct _HudApplicationSourceContext      HudApplicationSourceContext;
typedef struct _HudApplicationSourceContextClass HudApplicationSourceContextClass;

struct _HudApplicationSourceContextClass {
	GObjectClass parent_class;
};

struct _HudApplicationSourceContext {
	GObject parent;
};

GType hud_application_source_context_get_type (void);

G_END_DECLS

#endif
