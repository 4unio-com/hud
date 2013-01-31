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

#if !defined (_HUD_CLIENT_H_INSIDE) && !defined (HUD_CLIENT_COMPILATION)
#error "Only <hud-client.h> can be included directly."
#endif

#ifndef __HUD_CLIENT_PARAM_H__
#define __HUD_CLIENT_PARAM_H__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define HUD_CLIENT_TYPE_PARAM            (hud_client_param_get_type ())
#define HUD_CLIENT_PARAM(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), HUD_CLIENT_TYPE_PARAM, HudClientParam))
#define HUD_CLIENT_PARAM_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), HUD_CLIENT_TYPE_PARAM, HudClientParamClass))
#define HUD_CLIENT_IS_PARAM(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HUD_CLIENT_TYPE_PARAM))
#define HUD_CLIENT_IS_PARAM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), HUD_CLIENT_TYPE_PARAM))
#define HUD_CLIENT_PARAM_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), HUD_CLIENT_TYPE_PARAM, HudClientParamClass))

typedef struct _HudClientParam          HudClientParam;
typedef struct _HudClientParamClass     HudClientParamClass;
typedef struct _HudClientParamPrivate   HudClientParamPrivate;

/**
 * HudClientParamClass:
 * @parent_class: #GObjectClass
 *
 * Class information for #HudClientParam
 */
struct _HudClientParamClass {
	GObjectClass parent_class;
};

/**
 * HudClientParam:
 *
 * An object that tracks all the of the stuff needed to handle
 * a parameterized dialog of actions.
 */
struct _HudClientParam {
	GObject parent;
	HudClientParamPrivate * priv;
};

GType hud_client_param_get_type (void);

/**
	SECTION:param
	@short_description: Track a parameterized view
	@stability: Unstable
	@include: libhud-client/param.h

	This makes it much easier to interact with the parameterized
	pane of the HUD.  Provides the links to the menu model and the
	actions that should be shown.  Also provides convienience functions
	for resetting it and fun stuff like that.
*/

G_END_DECLS

#endif
