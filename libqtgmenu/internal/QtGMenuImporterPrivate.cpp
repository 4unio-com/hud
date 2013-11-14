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

#include <QEventLoop>

using namespace qtgmenu;

// this is used to suppress compiler warnings about unused parameters
template< typename ... T > void unused( T&& ... )
{
}

QtGMenuImporterPrivate::QtGMenuImporterPrivate( const QString& service, const QString& path,
    QtGMenuImporter& parent )
    : QObject( 0 ),
      m_parent( parent ),
      m_connection( g_bus_get_sync( G_BUS_TYPE_SESSION, NULL, NULL ) ),
      m_service( service.toStdString() ),
      m_path( path.toStdString() )
{
  connect( &m_menu_poll_timer, SIGNAL( timeout() ), this, SLOT( RefreshGMenuModel() ) );
  connect( &m_actions_poll_timer, SIGNAL( timeout() ), this, SLOT( RefreshGActionGroup() ) );
  StartPolling( 100 );
}

QtGMenuImporterPrivate::~QtGMenuImporterPrivate()
{
  m_menu_poll_timer.stop();
  m_actions_poll_timer.stop();

  g_object_unref( m_connection );

  ClearGActionGroup();
  ClearGMenuModel();
}

GMenuModel* QtGMenuImporterPrivate::GetGMenuModel()
{
  if( m_menu_poll_timer.isActive() || m_gmenu_model == nullptr )
  {
    return nullptr;
  }

  return m_gmenu_model->Model();
}

GActionGroup* QtGMenuImporterPrivate::GetGActionGroup()
{
  if( m_menu_poll_timer.isActive() || m_actions_poll_timer.isActive() || m_gmenu_model == nullptr )
  {
    return nullptr;
  }

  return m_gmenu_model->ActionGroup();
}

std::vector< QMenu* > QtGMenuImporterPrivate::GetQMenus()
{
  GMenuModel* gmenu = GetGMenuModel();

  if( gmenu == nullptr )
  {
    return std::vector< QMenu* >();
  }

  return m_gmenu_model->GetQMenus();
}

void QtGMenuImporterPrivate::StartPolling( int interval )
{
  m_menu_poll_timer.stop();
  m_menu_poll_timer.setInterval( interval );

  m_actions_poll_timer.stop();
  m_actions_poll_timer.setInterval( interval );

  // start polling for menu first
  m_menu_poll_timer.start();
}

void QtGMenuImporterPrivate::ClearGMenuModel()
{
  if( m_gmenu_model == nullptr )
  {
    return;
  }

  if( m_gmenu_model->Size() > 0 )
  {
    // notify parent that items are being removed
    emit m_parent.MenuItemsChanged();
  }

  disconnect( m_items_changed_conn );

  delete m_gmenu_model;
  m_gmenu_model = nullptr;
}

void QtGMenuImporterPrivate::ClearGActionGroup()
{
  if( m_gmenu_model != nullptr )
  {
    m_gmenu_model->SetActionGroup( nullptr );
  }

  disconnect( m_action_added_conn );
  disconnect( m_action_removed_conn );
  disconnect( m_action_enabled_conn );
  disconnect( m_action_state_changed_conn );
}

bool QtGMenuImporterPrivate::RefreshGMenuModel()
{
  bool menu_was_valid = m_gmenu_model != nullptr;
  GActionGroup* actions_backup = nullptr;

  if( menu_was_valid )
  {
    actions_backup = m_gmenu_model->ActionGroup();
    if( actions_backup )
    {
      g_object_ref( actions_backup );
    }
  }

  // clear the menu model for the refresh
  ClearGMenuModel();

  m_gmenu_model = new QtGMenuModel(
      G_MENU_MODEL( g_dbus_menu_model_get( m_connection, m_service.c_str(), m_path.c_str() ) ) );

  m_items_changed_conn = connect( m_gmenu_model, SIGNAL( MenuItemsChanged( QtGMenuModel*, int, int,
      int ) ), &m_parent, SIGNAL( MenuItemsChanged()) );

  gint item_count = m_gmenu_model->Size();

  if( item_count == 0 )
  {
    // block until "items-changed" fires or timeout reached
    QEventLoop menu_refresh_wait;
    QTimer timeout;

    menu_refresh_wait.connect( &m_parent, SIGNAL( MenuItemsChanged() ), SLOT( quit() ) );
    timeout.singleShot( 100, &menu_refresh_wait, SLOT( quit() ) );
    menu_refresh_wait.exec();

    // check item count again
    item_count = m_gmenu_model->Size();
  }

  bool menu_is_valid = item_count != 0;

  if( menu_is_valid )
  {
    m_gmenu_model->SetActionGroup( actions_backup );

    // menu is valid, start polling for actions
    m_menu_poll_timer.stop();
    m_actions_poll_timer.start();
  }
  else if( !menu_is_valid )
  {
    // clear the menu model
    ClearGMenuModel();
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
  bool actions_were_valid = m_gmenu_model->ActionGroup() != nullptr;

  // clear the action group for the refresh
  ClearGActionGroup();

  m_gmenu_model->SetActionGroup( G_ACTION_GROUP( g_dbus_action_group_get( m_connection,
      m_service.c_str(), m_path.c_str() ) ) );

  m_action_added_conn = connect( m_gmenu_model, SIGNAL( ActionAdded( QString ) ), &m_parent,
      SIGNAL( ActionAdded( QString ) ) );
  m_action_removed_conn = connect( m_gmenu_model, SIGNAL( ActionRemoved( QString ) ), &m_parent,
      SIGNAL( ActionRemoved( QString ) ) );
  m_action_enabled_conn = connect( m_gmenu_model, SIGNAL( ActionEnabled( QString, bool ) ),
      &m_parent, SIGNAL( ActionEnabled( QString, bool ) ) );
  m_action_state_changed_conn = connect( m_gmenu_model, SIGNAL( ActionStateChanged( QString,
      QVariant ) ), &m_parent, SIGNAL( ActionStateChanged( QString, QVariant) ) );

  int item_count = m_gmenu_model->ActionsCount();

  if( item_count == 0 )
  {
    // block until "items-changed" fires or timeout reached
    QEventLoop actions_refresh_wait;
    QTimer timeout;

    actions_refresh_wait.connect( &m_parent, SIGNAL( ActionAdded( QString ) ), SLOT( quit() ) );
    timeout.singleShot( 100, &actions_refresh_wait, SLOT( quit() ) );
    actions_refresh_wait.exec();

    // check item count again
    item_count = m_gmenu_model->ActionsCount();
  }

  bool actions_are_valid = item_count != 0;

  if( actions_are_valid )
  {
    // actions are valid, no need to continue polling
    m_actions_poll_timer.stop();
  }
  else if( !actions_are_valid )
  {
    // clear the action group
    ClearGActionGroup();
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
