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

QtGMenuImporterPrivate::QtGMenuImporterPrivate( const QString& service, const QString& path,
    QtGMenuImporter& parent )
    : QObject( 0 ),
      m_service_watcher( service, QDBusConnection::sessionBus(),
          QDBusServiceWatcher::WatchForOwnerChange ),
      m_parent( parent ),
      m_connection( g_bus_get_sync( G_BUS_TYPE_SESSION, NULL, NULL ) ),
      m_service( service.toStdString() ),
      m_path( path.toStdString() )
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

  g_object_unref( m_connection );

  ClearMenuModel();
  ClearActionGroup();
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
  GMenuModel* gmenu = GetGMenuModel();

  if( gmenu == nullptr )
  {
    return nullptr;
  }

  return m_menu_model->GetQMenu();
}

void QtGMenuImporterPrivate::StartPolling()
{
  m_menu_poll_timer.setInterval( 100 );
  m_menu_poll_timer.start();

  m_actions_poll_timer.setInterval( 100 );
  m_actions_poll_timer.start();
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

  UnlinkMenuActions();

  disconnect( m_items_changed_conn );

  delete m_menu_model;
  m_menu_model = nullptr;
}

void QtGMenuImporterPrivate::ClearActionGroup()
{
  if( m_action_group == nullptr )
  {
    return;
  }

  UnlinkMenuActions();

  disconnect( m_action_added_conn );
  disconnect( m_action_removed_conn );
  disconnect( m_action_enabled_conn );
  disconnect( m_action_state_changed_conn );

  delete m_action_group;
  m_action_group = nullptr;
}

void QtGMenuImporterPrivate::LinkMenuActions()
{
  if( m_menu_model && m_action_group && !m_menu_actions_linked )
  {
    m_action_activated_conn = connect( m_menu_model, SIGNAL( ActionTriggered( QString, bool ) ), m_action_group,
        SLOT( TriggerAction( QString, bool ) ) );

    m_menu_actions_linked = true;
  }
}

void QtGMenuImporterPrivate::UnlinkMenuActions()
{
  if( m_menu_actions_linked )
  {
    disconnect( m_action_activated_conn );

    m_menu_actions_linked = false;
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

  m_menu_model = new QtGMenuModel(
      G_MENU_MODEL( g_dbus_menu_model_get( m_connection, m_service.c_str(), m_path.c_str() ) ) );

  m_items_changed_conn = connect( m_menu_model, SIGNAL( MenuItemsChanged( QtGMenuModel*, int, int,
          int ) ), &m_parent, SIGNAL( MenuItemsChanged()) );

  gint item_count = m_menu_model->Size();

  if( item_count == 0 )
  {
    // block until "items-changed" fires or timeout reached
    QEventLoop menu_refresh_wait;
    QTimer timeout;

    menu_refresh_wait.connect( &m_parent, SIGNAL( MenuItemsChanged() ), SLOT( quit() ) );
    timeout.singleShot( 100, &menu_refresh_wait, SLOT( quit() ) );
    menu_refresh_wait.exec();

    // check item count again
    item_count = m_menu_model->Size();
  }

  bool menu_is_valid = item_count != 0;

  if( menu_is_valid )
  {
    // menu is valid, start polling for actions
    LinkMenuActions();
    m_menu_poll_timer.stop();
  }
  else if( !menu_is_valid )
  {
    // clear the menu model
    ClearMenuModel();
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

  m_action_group =
      new QtGActionGroup(
          G_ACTION_GROUP( g_dbus_action_group_get( m_connection, m_service.c_str(), m_path.c_str() ) ) );

  m_action_added_conn = connect( m_action_group, SIGNAL( ActionAdded( QString ) ), &m_parent,
      SIGNAL( ActionAdded( QString ) ) );
  m_action_removed_conn = connect( m_action_group, SIGNAL( ActionRemoved( QString ) ), &m_parent,
      SIGNAL( ActionRemoved( QString ) ) );
  m_action_enabled_conn = connect( m_action_group, SIGNAL( ActionEnabled( QString, bool ) ),
      &m_parent, SIGNAL( ActionEnabled( QString, bool ) ) );
  m_action_state_changed_conn = connect( m_action_group, SIGNAL( ActionStateChanged( QString,
          QVariant ) ), &m_parent, SIGNAL( ActionStateChanged( QString, QVariant) ) );

  int action_count = m_action_group->Size();

  if( action_count == 0 )
  {
    // block until "items-changed" fires or timeout reached
    QEventLoop actions_refresh_wait;
    QTimer timeout;

    actions_refresh_wait.connect( &m_parent, SIGNAL( ActionAdded( QString ) ), SLOT( quit() ) );
    timeout.singleShot( 100, &actions_refresh_wait, SLOT( quit() ) );
    actions_refresh_wait.exec();

    // check item count again
    action_count = m_action_group->Size();
  }

  bool actions_are_valid = action_count != 0;

  if( actions_are_valid )
  {
    // actions are valid, no need to continue polling
    LinkMenuActions();
    m_actions_poll_timer.stop();
  }
  else if( !actions_are_valid )
  {
    // clear the action group
    ClearActionGroup();
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
