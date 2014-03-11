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

#include <QtGMenuImporterPrivate.h>
#include <QtGMenuUtils.h>

#include <QDBusConnection>
#include <QEventLoop>

using namespace qtgmenu;

QtGMenuImporterPrivate::QtGMenuImporterPrivate( const QString& service, const QDBusObjectPath& menu_path,
                                                const QMap<QString, QDBusObjectPath>& action_paths, QtGMenuImporter& parent )
    : QObject( 0 ),
      m_service_watcher( service, QDBusConnection::sessionBus(),
          QDBusServiceWatcher::WatchForOwnerChange ),
      m_parent( parent ),
      m_connection( g_bus_get_sync( G_BUS_TYPE_SESSION, NULL, NULL ) ),
      m_service( service.toStdString() ),
      m_menu_path( menu_path.path().toStdString() )
{
  // we need to store action paths as std::strings because G_ACTION_GROUP requires C strings.
  // using QString::toStdString().c_str() is not safe as the std::string created is a temporary.
  for( auto const& action_path : action_paths.toStdMap() )
  {
    m_action_paths[action_path.first] = action_path.second.path().toStdString();
  }

  connect( &m_service_watcher, SIGNAL( serviceRegistered( const QString& ) ), this,
      SLOT( ServiceRegistered() ) );

  connect( &m_service_watcher, SIGNAL( serviceUnregistered( const QString& ) ), this,
      SLOT( ServiceUnregistered() ) );

  Refresh();
}

QtGMenuImporterPrivate::~QtGMenuImporterPrivate()
{
  ClearMenuModel();
  ClearActionGroups();

  g_object_unref( m_connection );
}

GMenuModel* QtGMenuImporterPrivate::GetGMenuModel()
{
  if( m_menu_model == nullptr )
  {
    return nullptr;
  }

  return m_menu_model->Model();
}

GActionGroup* QtGMenuImporterPrivate::GetGActionGroup( int index )
{
  if( index >= m_action_groups.size() ||
      m_action_groups[index] == nullptr )
  {
    return nullptr;
  }

  return m_action_groups[index]->ActionGroup();
}

std::shared_ptr< QMenu > QtGMenuImporterPrivate::GetQMenu()
{
  if( m_menu_model == nullptr )
  {
    return nullptr;
  }

  return m_menu_model->GetQMenu();
}

void QtGMenuImporterPrivate::Refresh()
{
  if( !m_menu_path.empty() )
  {
    RefreshGMenuModel();
  }

  if( !m_action_paths.empty() )
  {
    RefreshGActionGroup();
  }
}

void QtGMenuImporterPrivate::ClearMenuModel()
{
  if( m_menu_model == nullptr )
  {
    return;
  }

  m_menu_model->disconnect();
  m_menu_actions_linked = false;

  m_menu_model = nullptr;
}

void QtGMenuImporterPrivate::ClearActionGroups()
{
  for( auto& action_group : m_action_groups )
  {
    action_group->disconnect();
  }

  m_menu_actions_linked = false;
  m_action_groups.clear();
}

void QtGMenuImporterPrivate::LinkMenuActions()
{
  if( m_menu_model && !m_action_groups.empty() && !m_menu_actions_linked )
  {
    for( auto& action_group : m_action_groups )
    {
      connect( m_menu_model.get(), SIGNAL( ActionTriggered( QString, bool ) ), action_group.get(),
          SLOT( TriggerAction( QString, bool ) ) );

      connect( m_menu_model.get(), SIGNAL( MenuItemsChanged( QtGMenuModel*, int, int, int ) ),
          action_group.get(), SLOT( EmitStates() ) );

      connect( action_group.get(), SIGNAL( ActionEnabled( QString, bool ) ), m_menu_model.get(),
          SLOT( ActionEnabled( QString, bool ) ) );

      connect( action_group.get(), SIGNAL( ActionParameterized( QString, bool ) ),
          m_menu_model.get(), SLOT( ActionParameterized( QString, bool ) ) );
    }

    m_menu_actions_linked = true;
  }
}

void QtGMenuImporterPrivate::ServiceRegistered()
{
  Refresh();
}

void QtGMenuImporterPrivate::ServiceUnregistered()
{
  ClearMenuModel();
  ClearActionGroups();
}

void QtGMenuImporterPrivate::RefreshGMenuModel()
{
  // clear the menu model for the refresh
  ClearMenuModel();

  m_menu_model =
      std::make_shared< QtGMenuModel > (
              G_MENU_MODEL( g_dbus_menu_model_get( m_connection, m_service.c_str(), m_menu_path.c_str() ) ),
              m_service.c_str(), m_menu_path.c_str() );

  connect( m_menu_model.get(), SIGNAL( MenuItemsChanged( QtGMenuModel*, int, int,
          int ) ), &m_parent, SIGNAL( MenuItemsChanged()) );
}

void QtGMenuImporterPrivate::RefreshGActionGroup()
{
  // clear the action groups for the refresh
  ClearActionGroups();

  for( auto const& action_path : m_action_paths )
  {
    m_action_groups.push_back(
        std::make_shared< QtGActionGroup > ( action_path.first,
                G_ACTION_GROUP( g_dbus_action_group_get( m_connection, m_service.c_str(), action_path.second.c_str() ) ) ) );

    auto action_group = m_action_groups.back();

    connect( action_group.get(), SIGNAL( ActionAdded( QString ) ), &m_parent,
        SIGNAL( ActionAdded( QString ) ) );
    connect( action_group.get(), SIGNAL( ActionRemoved( QString ) ), &m_parent,
        SIGNAL( ActionRemoved( QString ) ) );
    connect( action_group.get(), SIGNAL( ActionEnabled( QString, bool ) ), &m_parent,
        SIGNAL( ActionEnabled( QString, bool ) ) );
    connect( action_group.get(), SIGNAL( ActionStateChanged( QString,
            QVariant ) ), &m_parent, SIGNAL( ActionStateChanged( QString, QVariant) ) );
  }

  LinkMenuActions();
}
