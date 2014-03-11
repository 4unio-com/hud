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
      m_menu_path( menu_path.path().toStdString() ),
      m_actions_path( action_paths.cbegin().value().path().toStdString() )
{
  connect( &m_service_watcher, SIGNAL( serviceRegistered( const QString& ) ), this,
      SLOT( ServiceRegistered() ) );

  connect( &m_service_watcher, SIGNAL( serviceUnregistered( const QString& ) ), this,
      SLOT( ServiceUnregistered() ) );

  Refresh();
}

QtGMenuImporterPrivate::~QtGMenuImporterPrivate()
{
  ClearMenuModel();
  ClearActionGroup();

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

GActionGroup* QtGMenuImporterPrivate::GetGActionGroup()
{
  if( m_action_group == nullptr )
  {
    return nullptr;
  }

  return m_action_group->ActionGroup();
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

  if( !m_actions_path.empty() )
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

void QtGMenuImporterPrivate::ClearActionGroup()
{
  if( m_action_group == nullptr )
  {
    return;
  }

  m_action_group->disconnect();
  m_menu_actions_linked = false;

  m_action_group = nullptr;
}

void QtGMenuImporterPrivate::LinkMenuActions()
{
  if( m_menu_model && m_action_group && !m_menu_actions_linked )
  {
    connect( m_menu_model.get(), SIGNAL( ActionTriggered( QString, bool ) ), m_action_group.get(),
        SLOT( TriggerAction( QString, bool ) ) );

    connect( m_menu_model.get(), SIGNAL( MenuItemsChanged( QtGMenuModel*, int, int, int ) ),
        m_action_group.get(), SLOT( EmitStates() ) );

    connect( m_action_group.get(), SIGNAL( ActionEnabled( QString, bool ) ), m_menu_model.get(),
        SLOT( ActionEnabled( QString, bool ) ) );

    connect( m_action_group.get(), SIGNAL( ActionParameterized( QString, bool ) ),
        m_menu_model.get(), SLOT( ActionParameterized( QString, bool ) ) );

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
  ClearActionGroup();
}

void QtGMenuImporterPrivate::RefreshGMenuModel()
{
  // clear the menu model for the refresh
  ClearMenuModel();

  m_menu_model =
      std::make_shared< QtGMenuModel > (
              G_MENU_MODEL( g_dbus_menu_model_get( m_connection, m_service.c_str(), m_menu_path.c_str() ) ),
              m_service.c_str(), m_menu_path.c_str(), m_actions_path.c_str() );

  connect( m_menu_model.get(), SIGNAL( MenuItemsChanged( QtGMenuModel*, int, int,
          int ) ), &m_parent, SIGNAL( MenuItemsChanged()) );
}

void QtGMenuImporterPrivate::RefreshGActionGroup()
{
  // clear the action group for the refresh
  ClearActionGroup();

  m_action_group =
      std::make_shared< QtGActionGroup > (
              G_ACTION_GROUP( g_dbus_action_group_get( m_connection, m_service.c_str(), m_actions_path.c_str() ) ) );

  connect( m_action_group.get(), SIGNAL( ActionAdded( QString ) ), &m_parent,
      SIGNAL( ActionAdded( QString ) ) );
  connect( m_action_group.get(), SIGNAL( ActionRemoved( QString ) ), &m_parent,
      SIGNAL( ActionRemoved( QString ) ) );
  connect( m_action_group.get(), SIGNAL( ActionEnabled( QString, bool ) ), &m_parent,
      SIGNAL( ActionEnabled( QString, bool ) ) );
  connect( m_action_group.get(), SIGNAL( ActionStateChanged( QString,
          QVariant ) ), &m_parent, SIGNAL( ActionStateChanged( QString, QVariant) ) );

  LinkMenuActions();
}
