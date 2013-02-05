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

#else

typedef struct _dummyWindow AbstractWindow;
typedef struct _dummyApplication AbstractApplication;

struct _dummyWindow {
	int dummy;
};

struct _dummyApplication {
	int dummy;
};

#endif /* HAVE_BAMF */

#endif /* __HUD_ABSTRACT_APP_H__ */
