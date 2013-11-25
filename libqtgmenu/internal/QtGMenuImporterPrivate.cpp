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

// this is used to suppress compiler warnings about unused parameters
template< typename ... T > void unused( T&& ... )
{
}

QtGMenuImporterPrivate::QtGMenuImporterPrivate( const QString& service, const QString& menu_path,
    const QString& actions_path, QtGMenuImporter& parent )
    : QObject( 0 ),
      m_service_watcher( service, QDBusConnection::sessionBus(),
          QDBusServiceWatcher::WatchForOwnerChange ),
      m_parent( parent ),
      m_connection( g_bus_get_sync( G_BUS_TYPE_SESSION, NULL, NULL ) ),
      m_service( service.toStdString() ),
      m_menu_path( menu_path.toStdString() ),
      m_actions_path( actions_path.toStdString() )
{
  connect( &m_service_watcher, SIGNAL( serviceRegistered( const QString& ) ), this,
      SLOT( ServiceRegistered() ) );

  connect( &m_service_watcher, SIGNAL( serviceUnregistered( const QString& ) ), this,
      SLOT( ServiceUnregistered() ) );

  connect( &m_menu_poll_timer, SIGNAL( timeout() ), this, SLOT( RefreshGMenuModel() ) );
  connect( &m_actions_poll_timer, SIGNAL( timeout() ), this, SLOT( RefreshGActionGroup() ) );

  StartPolling();
}

QtGMenuImporterPrivate::~QtGMenuImporterPrivate()
{
  StopPolling();

  ClearMenuModel();
  ClearActionGroup();

  g_object_unref( m_connection );
}

GMenuModel* QtGMenuImporterPrivate::GetGMenuModel()
{
  if( m_menu_poll_timer.isActive() || m_menu_model == nullptr )
  {
    return nullptr;
  }

  return m_menu_model->Model();
}

GActionGroup* QtGMenuImporterPrivate::GetGActionGroup()
{
  if( m_actions_poll_timer.isActive() || m_action_group == nullptr )
  {
    return nullptr;
  }

  return m_action_group->ActionGroup();
}

std::shared_ptr< QMenu > QtGMenuImporterPrivate::GetQMenu()
{
  if( m_menu_poll_timer.isActive() || m_menu_model == nullptr )
  {
    return nullptr;
  }

  return m_menu_model->GetQMenu();
}

void QtGMenuImporterPrivate::StartPolling()
{
  if( !m_menu_path.empty() )
  {
    m_menu_poll_timer.setInterval( 100 );
    m_menu_poll_timer.start();
  }

  if( !m_actions_path.empty() )
  {
    m_actions_poll_timer.setInterval( 100 );
    m_actions_poll_timer.start();
  }
}

void QtGMenuImporterPrivate::StopPolling()
{
  m_menu_poll_timer.stop();
  m_actions_poll_timer.stop();
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
        m_action_group.get(), SLOT( RefreshStates() ) );

    connect( m_action_group.get(), SIGNAL( ActionEnabled( QString, bool ) ), m_menu_model.get(),
        SLOT( ActionEnabled( QString, bool ) ) );

    connect( m_action_group.get(), SIGNAL( ActionParameterized( QString, bool ) ),
        m_menu_model.get(), SLOT( ActionParameterized( QString, bool ) ) );

    m_menu_actions_linked = true;
  }
}

void QtGMenuImporterPrivate::ServiceRegistered()
{
  StartPolling();
}

void QtGMenuImporterPrivate::ServiceUnregistered()
{
  StopPolling();

  ClearMenuModel();
  ClearActionGroup();
}

bool QtGMenuImporterPrivate::RefreshGMenuModel()
{
  bool menu_was_valid = m_menu_model != nullptr;

  // clear the menu model for the refresh
  ClearMenuModel();

  auto menu_model =
      std::shared_ptr < QtGMenuModel
          > ( new QtGMenuModel(
              G_MENU_MODEL( g_dbus_menu_model_get( m_connection, m_service.c_str(), m_menu_path.c_str() ) ),
              m_service.c_str(), m_menu_path.c_str(), m_actions_path.c_str() ) );

  connect( menu_model.get(), SIGNAL( MenuItemsChanged( QtGMenuModel*, int, int,
          int ) ), &m_parent, SIGNAL( MenuItemsChanged()) );

  gint item_count = menu_model->Size();

  if( item_count == 0 )
  {
    // block until "items-changed" fires or timeout reached
    QEventLoop menu_refresh_wait;
    QTimer timeout;

    menu_refresh_wait.connect( &m_parent, SIGNAL( MenuItemsChanged() ), SLOT( quit() ) );
    timeout.singleShot( 100, &menu_refresh_wait, SLOT( quit() ) );
    menu_refresh_wait.exec();

    // check item count again
    item_count = menu_model->Size();
  }

  bool menu_is_valid = item_count != 0;

  if( menu_is_valid )
  {
    // menu is valid, start polling for actions
    m_menu_poll_timer.stop();

    m_menu_model = menu_model;
    LinkMenuActions();
  }

  if( !menu_was_valid && menu_is_valid )
  {
    emit m_parent.MenuAppeared();
  }
  else if( menu_was_valid && !menu_is_valid )
  {
    emit m_parent.MenuDisappeared();
  }

  return menu_is_valid;
}

bool QtGMenuImporterPrivate::RefreshGActionGroup()
{
  bool actions_were_valid = m_action_group != nullptr;

  // clear the action group for the refresh
  ClearActionGroup();

  auto action_group =
      std::shared_ptr < QtGActionGroup
          > ( new QtGActionGroup(
              G_ACTION_GROUP( g_dbus_action_group_get( m_connection, m_service.c_str(), m_actions_path.c_str() ) ) ) );

  connect( action_group.get(), SIGNAL( ActionAdded( QString ) ), &m_parent,
      SIGNAL( ActionAdded( QString ) ) );
  connect( action_group.get(), SIGNAL( ActionRemoved( QString ) ), &m_parent,
      SIGNAL( ActionRemoved( QString ) ) );
  connect( action_group.get(), SIGNAL( ActionEnabled( QString, bool ) ), &m_parent,
      SIGNAL( ActionEnabled( QString, bool ) ) );
  connect( action_group.get(), SIGNAL( ActionStateChanged( QString,
          QVariant ) ), &m_parent, SIGNAL( ActionStateChanged( QString, QVariant) ) );

  int action_count = action_group->Size();

  if( action_count == 0 )
  {
    // block until "items-changed" fires or timeout reached
    QEventLoop actions_refresh_wait;
    QTimer timeout;

    actions_refresh_wait.connect( &m_parent, SIGNAL( ActionAdded( QString ) ), SLOT( quit() ) );
    timeout.singleShot( 100, &actions_refresh_wait, SLOT( quit() ) );
    actions_refresh_wait.exec();

    // check item count again
    action_count = action_group->Size();
  }

  bool actions_are_valid = action_count != 0;

  if( actions_are_valid )
  {
    // actions are valid, no need to continue polling
    m_actions_poll_timer.stop();

    m_action_group = action_group;
    LinkMenuActions();
  }

  if( !actions_were_valid && actions_are_valid )
  {
    emit m_parent.ActionsAppeared();
  }
  else if( actions_were_valid && !actions_are_valid )
  {
    emit m_parent.ActionsDisappeared();
  }

  return actions_are_valid;
}

#include <QtGMenuImporterPrivate.moc>
