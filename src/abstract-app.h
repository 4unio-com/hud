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

#include "config.h"

#ifdef HAVE_BAMF

#include <libbamf/libbamf.h>

typedef BamfWindow AbstractWindow;
typedef BamfApplication AbstractApplication;

#endif /* HAVE_BAMF */

#ifdef HAVE_HYBRIS

#include <ubuntu/ui/ubuntu_ui_session_service.h>
// TODO ubuntu_application_ui.h is not C compiler friendly
// #include <ubuntu/application/ui/ubuntu_application_ui.h>

#define WINDOW_ID_CONSTANT (5)

typedef ubuntu_ui_session_properties AbstractWindow;
typedef ubuntu_ui_session_properties AbstractApplication;

#define _ubuntu_ui_session_properties_get_window_id(props) WINDOW_ID_CONSTANT

#endif /* HAVE_HYBRIS */

#endif /* __HUD_ABSTRACT_APP_H__ */
