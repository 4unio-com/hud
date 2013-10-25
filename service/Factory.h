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

#ifndef HUD_SERVICE_FACTORY_H_
#define HUD_SERVICE_FACTORY_H_

#include <HudService.h>
#include <Application.h>
#include <ApplicationList.h>
#include <Query.h>

namespace hud {
namespace service {

class Factory {
public:
	Factory();

	virtual ~Factory();

	virtual HudService::Ptr singletonHudService();

	virtual Query::Ptr newQuery(unsigned int id, const QString &query);

	virtual ApplicationList::Ptr newApplicationList();

	virtual Application::Ptr newApplication(unsigned int id,
			const QString &applicationId);

protected:
	HudService::Ptr m_hudService;
};

}
}

#endif /* HUD_SERVICE_FACTORY_H_ */
