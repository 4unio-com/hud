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

#include <QtGActionGroup.h>
#include <QtGMenuUtils.h>

using namespace qtgmenu;

QtGActionGroup::QtGActionGroup( GActionGroup* action_group )
    : m_action_group( action_group )
{
  ConnectCallbacks();
}

QtGActionGroup::~QtGActionGroup()
{
  if( m_action_group == nullptr )
  {
    return;
  }

  for( int i = 0; i < Size(); i++ )
  {
    emit ActionRemoved( Action( i ) );
  }

  DisconnectCallbacks();

  g_object_unref( m_action_group );
  m_action_group = nullptr;

  return;
}

GActionGroup* QtGActionGroup::ActionGroup() const
{
  return m_action_group;
}

int QtGActionGroup::Size()
{
  gchar** actions_list = g_action_group_list_actions( m_action_group );

  if( !actions_list )
  {
    return 0;
  }

  int action_count = 0;
  while( actions_list[action_count] != nullptr )
  {
    ++action_count;
  }

  g_strfreev( actions_list );

  return action_count;
}

QString QtGActionGroup::Action( int index )
{
  if( index >= Size() )
  {
    return QString();
  }

  QString action_name;
  gchar** actions_list = g_action_group_list_actions( m_action_group );

  action_name = QString( actions_list[index] );

  g_strfreev( actions_list );

  return action_name;
}

void QtGActionGroup::TriggerAction( QString action_name, bool checked )
{
  std::string action = action_name.toStdString();

  const GVariantType* type = g_action_group_get_action_parameter_type( m_action_group,
      action.c_str() );

  if( type == nullptr )
  {
    g_action_group_activate_action( m_action_group, action.c_str(), nullptr );
  }
  else
  {
    ///! need to evaluate and send parameter value
    if( g_variant_type_equal( type, G_VARIANT_TYPE_STRING ) )
    {
      GVariant* param = g_variant_new_string( action.c_str() );
      g_action_group_activate_action( m_action_group, action.c_str(), param );
    }
  }
}

void QtGActionGroup::RefreshStates()
{
  gchar** actions_list = g_action_group_list_actions( m_action_group );

  for( int i = 0; i < Size(); ++i )
  {
    gchar* action_name = actions_list[i];

    bool enabled = G_ACTION_GROUP_GET_IFACE( m_action_group ) ->get_action_enabled( m_action_group,
        action_name );
    emit ActionEnabled( action_name, enabled );

    const GVariantType* type = g_action_group_get_action_parameter_type( m_action_group,
        action_name );
    emit ActionParameterized( action_name, type != nullptr );
  }

  g_strfreev( actions_list );
}

void QtGActionGroup::ActionAddedCallback( GActionGroup* action_group, gchar* action_name,
    gpointer user_data )
{
  QtGActionGroup* self = reinterpret_cast< QtGActionGroup* >( user_data );
  emit self->ActionAdded( action_name );

  bool enabled = G_ACTION_GROUP_GET_IFACE( self->m_action_group ) ->get_action_enabled(
      self->m_action_group, action_name );
  emit self->ActionEnabled( action_name, enabled );

  const GVariantType* type = g_action_group_get_action_parameter_type( self->m_action_group,
      action_name );
  emit self->ActionParameterized( action_name, type != nullptr );
}

void QtGActionGroup::ActionRemovedCallback( GActionGroup* action_group, gchar* action_name,
    gpointer user_data )
{
  QtGActionGroup* self = reinterpret_cast< QtGActionGroup* >( user_data );
  emit self->ActionRemoved( action_name );
}

void QtGActionGroup::ActionEnabledCallback( GActionGroup* action_group, gchar* action_name,
    gboolean enabled, gpointer user_data )
{
  QtGActionGroup* self = reinterpret_cast< QtGActionGroup* >( user_data );
  emit self->ActionEnabled( action_name, enabled );
}

void QtGActionGroup::ActionStateChangedCallback( GActionGroup* action_group, gchar* action_name,
    GVariant* value, gpointer user_data )
{
  QtGActionGroup* self = reinterpret_cast< QtGActionGroup* >( user_data );
  emit self->ActionStateChanged( action_name, QtGMenuUtils::GVariantToQVariant( value ) );
}

void QtGActionGroup::ConnectCallbacks()
{
  if( m_action_group && m_action_added_handler == 0 )
  {
    m_action_added_handler = g_signal_connect( m_action_group, "action-added",
        G_CALLBACK( ActionAddedCallback ), this );
  }
  if( m_action_group && m_action_removed_handler == 0 )
  {
    m_action_removed_handler = g_signal_connect( m_action_group, "action-removed",
        G_CALLBACK( ActionRemovedCallback ), this );
  }
  if( m_action_group && m_action_enabled_handler == 0 )
  {
    m_action_enabled_handler = g_signal_connect( m_action_group, "action-enabled-changed",
        G_CALLBACK( ActionEnabledCallback ), this );
  }
  if( m_action_group && m_action_state_changed_handler == 0 )
  {
    m_action_state_changed_handler = g_signal_connect( m_action_group, "action-state-changed",
        G_CALLBACK( ActionStateChangedCallback ), this );
  }
}

void QtGActionGroup::DisconnectCallbacks()
{
  if( m_action_group && m_action_added_handler != 0 )
  {
    g_signal_handler_disconnect( m_action_group, m_action_added_handler );
  }
  if( m_action_group && m_action_removed_handler != 0 )
  {
    g_signal_handler_disconnect( m_action_group, m_action_removed_handler );
  }
  if( m_action_group && m_action_enabled_handler != 0 )
  {
    g_signal_handler_disconnect( m_action_group, m_action_enabled_handler );
  }
  if( m_action_group && m_action_state_changed_handler != 0 )
  {
    g_signal_handler_disconnect( m_action_group, m_action_state_changed_handler );
  }

  m_action_added_handler = 0;
  m_action_removed_handler = 0;
  m_action_enabled_handler = 0;
  m_action_state_changed_handler = 0;
}

#include <QtGActionGroup.moc>
