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

#ifndef __HUD_ABSTRACT_APP_H__
#define __HUD_ABSTRACT_APP_H__

#define WINDOW_ID_ALL_WINDOWS (0)

#ifdef HAVE_BAMF

#include <libbamf/libbamf.h>

typedef BamfWindow AbstractWindow;
typedef BamfApplication AbstractApplication;

#define abstract_window_get_id(win)  bamf_window_get_xid(win)

#endif /* HAVE_BAMF */

#ifdef HAVE_PLATFORM_API

#include <glib.h>

#define WINDOW_ID_CONSTANT (5)

typedef struct {
  gchar *desktop_file_hint;
  int    stage_hint;
} SessionProperties;

typedef SessionProperties AbstractWindow;
typedef SessionProperties AbstractApplication;

#define session_properties_get_window_id(props) WINDOW_ID_CONSTANT

#define abstract_window_get_id(win)  WINDOW_ID_CONSTANT

#endif /* HAVE_PLATFORM_API */

#endif /* __HUD_ABSTRACT_APP_H__ */
