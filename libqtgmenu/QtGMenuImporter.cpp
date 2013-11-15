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
 * Author: Marcus Tomlinson <marcus.tomlinson@canonical.com>
 */

#include <QtGMenuImporter.h>
#include <internal/QtGMenuImporterPrivate.h>

#include <QMenu>

using namespace qtgmenu;

QtGMenuImporter::QtGMenuImporter( const QString &service, const QString &path, QObject* parent )
    : QObject( parent ),
      d( new QtGMenuImporterPrivate( service, path, *this ) )
{
}

QtGMenuImporter::~QtGMenuImporter()
{
}

GMenuModel* QtGMenuImporter::GetGMenuModel() const
{
  return d->GetGMenuModel();
}

GActionGroup* QtGMenuImporter::GetGActionGroup() const
{
  return d->GetGActionGroup();
}

std::vector< QMenu* > QtGMenuImporter::GetQMenus() const
{
  return d->GetQMenus();
}

void QtGMenuImporter::ForceRefresh()
{
  d->StartPolling();
}

#include <QtGMenuImporter.moc>
