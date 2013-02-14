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

#ifndef __APP_LIST_DUMMY_H__
#define __APP_LIST_DUMMY_H__

#include "application-list.h"
#include <glib-object.h>

G_BEGIN_DECLS

#define APP_LIST_DUMMY_TYPE            (app_list_dummy_get_type ())
#define APP_LIST_DUMMY(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), APP_LIST_DUMMY_TYPE, AppListDummy))
#define APP_LIST_DUMMY_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), APP_LIST_DUMMY_TYPE, AppListDummyClass))
#define IS_APP_LIST_DUMMY(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), APP_LIST_DUMMY_TYPE))
#define IS_APP_LIST_DUMMY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), APP_LIST_DUMMY_TYPE))
#define APP_LIST_DUMMY_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), APP_LIST_DUMMY_TYPE, AppListDummyClass))

typedef struct _AppListDummy      AppListDummy;
typedef struct _AppListDummyClass AppListDummyClass;

struct _AppListDummyClass {
	HudApplicationListClass parent_class;
};

struct _AppListDummy {
	HudApplicationList parent;
};

GType app_list_dummy_get_type (void);
AppListDummy * app_list_dummy_new (HudSource * focused_source);
void app_list_dummy_set_focus (AppListDummy * dummy, HudSource * focused_source);

G_END_DECLS

#endif
