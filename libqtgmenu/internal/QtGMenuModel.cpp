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

#include <QtGMenuModel.h>
#include <QtGMenuUtils.h>

using namespace qtgmenu;

QtGMenuModel::QtGMenuModel( GMenuModel* model )
    : QtGMenuModel( model, LinkType::Root, nullptr, 0 )
{
}

QtGMenuModel::QtGMenuModel( GMenuModel* model, LinkType link_type, QtGMenuModel* parent,
    int position )
    : m_model( model ),
      m_parent( parent ),
      m_link_type( link_type )
{
  // we should always have at least one section in a menu
  m_sections.push_back( std::deque< QObject* >() );

  if( m_parent )
  {
    m_parent->InsertChild( this, position );

    GVariant* label = g_menu_model_get_item_attribute_value( m_parent->m_model, position,
        G_MENU_ATTRIBUTE_LABEL, G_VARIANT_TYPE_STRING );

    if( label )
    {
      QString qlabel = QtGMenuUtils::GVariantToQVariant( label ).toString();
      qlabel.replace( '_', '&' );
      g_variant_unref( label );

      m_menu->setTitle( qlabel );
    }
  }

  if( m_model )
  {
    m_size = g_menu_model_get_n_items( m_model );
  }

  for( int i = 0; i < m_size; ++i )
  {
    CreateModel( this, m_model, i );
  }

  ConnectMenuCallback();
}

QtGMenuModel::~QtGMenuModel()
{
  SetActionGroup( nullptr );

  if( m_model )
  {
    //MenuItemsChangedCallback( m_model, 0, Size(), 0, this );
    g_object_unref( m_model );
  }
  DisconnectMenuCallback();

  for( auto child : m_children )
  {
    delete child;
  }
  m_children.clear();

  delete m_menu;
}

GMenuModel* QtGMenuModel::Model() const
{
  return m_model;
}

QtGMenuModel::LinkType QtGMenuModel::Type() const
{
  return m_link_type;
}

int QtGMenuModel::Size() const
{
  return m_size;
}

QtGMenuModel* QtGMenuModel::Parent() const
{
  return m_parent;
}

QtGMenuModel* QtGMenuModel::Child( int position ) const
{
  if( m_children.contains( position ) )
  {
    return m_children.value( position );
  }

  return nullptr;
}

std::vector< QMenu* > QtGMenuModel::GetQMenus()
{
  std::vector< QMenu* > menus;

  AppendQMenu( menus );

  return menus;
}

GActionGroup* QtGMenuModel::ActionGroup() const
{
  return m_action_group;
}

void QtGMenuModel::SetActionGroup( GActionGroup* action_group )
{
  if( action_group == nullptr )
  {
    if( m_action_group == nullptr )
    {
      return;
    }

    for( int i = 0; i < ActionsCount(); i++ )
    {
      emit ActionRemoved( ActionAt( i ) );
    }

    DisconnectActionCallbacks();

    g_object_unref( m_action_group );
    m_action_group = nullptr;

    return;
  }

  m_action_group = action_group;

  ConnectActionCallbacks();
}

QString QtGMenuModel::ActionAt( int position )
{
  if( position >= ActionsCount() )
  {
    return QString();
  }

  QString action_name;
  gchar** actions_list = g_action_group_list_actions( m_action_group );

  action_name = QString( actions_list[ position ] );

  g_strfreev( actions_list );

  return action_name;
}

int QtGMenuModel::ActionsCount()
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

void QtGMenuModel::MenuItemsChangedCallback( GMenuModel* model, gint position, gint removed,
    gint added, gpointer user_data )
{
  QtGMenuModel* self = reinterpret_cast< QtGMenuModel* >( user_data );
  self->ChangeMenuItems( position, added, removed );
}

void QtGMenuModel::ActionAddedCallback( GActionGroup* action_group, gchar* action_name,
    gpointer user_data )
{
  QtGMenuModel* self = reinterpret_cast< QtGMenuModel* >( user_data );
  emit self->ActionAdded( action_name );
}

void QtGMenuModel::ActionRemovedCallback( GActionGroup* action_group, gchar* action_name,
    gpointer user_data )
{
  QtGMenuModel* self = reinterpret_cast< QtGMenuModel* >( user_data );
  emit self->ActionRemoved( action_name );
}

void QtGMenuModel::ActionEnabledCallback( GActionGroup* action_group, gchar* action_name,
    gboolean enabled, gpointer user_data )
{
  QtGMenuModel* self = reinterpret_cast< QtGMenuModel* >( user_data );
  emit self->ActionEnabled( action_name, enabled );
}

void QtGMenuModel::ActionStateChangedCallback( GActionGroup* action_group,
    gchar* action_name, GVariant* value, gpointer user_data )
{
  QtGMenuModel* self = reinterpret_cast< QtGMenuModel* >( user_data );
  emit self->ActionStateChanged( action_name, QtGMenuUtils::GVariantToQVariant( value ) );
}

void QtGMenuModel::ChangeMenuItems( int position, int added, int removed )
{
  auto model_effected = this;
  auto section_effected = &m_sections[0];
  if( m_link_type == LinkType::Section && m_parent )
  {
    model_effected = m_parent;
    section_effected = &m_parent->m_sections[ m_parent->ChildPosition( this ) ];
  }

  if( removed > 0 )
  {
    auto start_it = section_effected->begin() + position;
    auto end_it = start_it + removed;
    section_effected->erase( start_it, end_it );

    for( int i = position; i < m_size; ++i )
    {
      if( i <= ( position + removed ) )
      {
        delete m_children.take( i );
      }
      else if( m_children.contains( i ) )
      {
        m_children.insert( i - removed, m_children.take( i ) );
      }
    }

    m_size -= removed;
  }

  if( added > 0 )
  {
    // shift items up
    for( int i = ( m_size + added ) - 1; i >= position; --i )
    {
      if( m_children.contains( i ) )
      {
        m_children.insert( i + added, m_children.take( i ) );
      }
    }

    m_size += added;

    for( int i = position; i < ( position + added ); ++i )
    {
      QtGMenuModel* model = CreateModel( this, m_model, i );
      if( !model )
      {
        auto it = section_effected->begin();
        section_effected->insert( it + i, 1, CreateAction( i ) );
      }
      else if( model->Type() == LinkType::SubMenu )
      {
        auto it = section_effected->begin();
        section_effected->insert( it + i, 1, model->m_menu );
      }
    }
  }

  model_effected->RefreshQMenu();
  emit MenuItemsChanged( this, position, removed, added );
}

QtGMenuModel* QtGMenuModel::CreateModel( QtGMenuModel* parent, GMenuModel* model, int position )
{
  LinkType linkType( LinkType::SubMenu );
  GMenuModel* link = g_menu_model_get_item_link( model, position, G_MENU_LINK_SUBMENU );

  if( !link )
  {
    linkType = LinkType::Section;
    link = g_menu_model_get_item_link( model, position, G_MENU_LINK_SECTION );
  }

  if( link )
  {
    return new QtGMenuModel( link, linkType, parent, position );
  }

  return nullptr;
}

void QtGMenuModel::ConnectMenuCallback()
{
  if( m_model && m_items_changed_handler == 0 )
  {
    m_items_changed_handler = g_signal_connect( m_model, "items-changed",
        G_CALLBACK( MenuItemsChangedCallback ), this );
  }
}

void QtGMenuModel::DisconnectMenuCallback()
{
  if( m_model && m_items_changed_handler != 0 )
  {
    g_signal_handler_disconnect( m_model, m_items_changed_handler );
  }

  m_items_changed_handler = 0;
}

void QtGMenuModel::ConnectActionCallbacks()
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

void QtGMenuModel::DisconnectActionCallbacks()
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

void QtGMenuModel::InsertChild( QtGMenuModel* child, int position )
{
  if( m_children.contains( position ) )
  {
    return;
  }

  if( child->Type() == LinkType::Section )
  {
    auto it = m_sections.begin();
    m_sections.insert( it + position, 1, std::deque< QObject* >() );
  }

  child->m_parent = this;
  m_children.insert( position, child );

  connect( child, SIGNAL( MenuItemsChanged(QtGMenuModel*,int,int,int)), this,
      SIGNAL( MenuItemsChanged(QtGMenuModel*,int,int,int)) );
}

int QtGMenuModel::ChildPosition( QtGMenuModel* child )
{
  for( int i = 0; i < m_children.size(); ++i )
  {
    if( child == m_children[i] )
    {
      return i;
    }
  }

  return -1;
}

QAction* QtGMenuModel::CreateAction( int position )
{
  QAction* action = new QAction( this );

  GVariant* label = g_menu_model_get_item_attribute_value( m_model, position,
      G_MENU_ATTRIBUTE_LABEL, G_VARIANT_TYPE_STRING );

  GVariant* icon = g_menu_model_get_item_attribute_value( m_model, position,
      G_MENU_ATTRIBUTE_ICON, G_VARIANT_TYPE_VARIANT );

  QString qlabel = QtGMenuUtils::GVariantToQVariant( label ).toString();
  qlabel.replace( '_', '&' );
  g_variant_unref( label );

  action->setEnabled( true );

  action->setText( qlabel );
  return action;
}

void QtGMenuModel::AppendQMenu( std::vector< QMenu* >& menus )
{
  menus.push_back( m_menu );

  if( m_link_type != LinkType::SubMenu )
  {
    for( auto& child : m_children )
    {
      child->AppendQMenu( menus );
    }
  }
}

void QtGMenuModel::RefreshQMenu()
{
  QAction* action = nullptr;
  QMenu* menu = nullptr;
  QAction* last_separator = nullptr;

  m_menu->clear();

  for( auto& section : m_sections )
  {
    if( section.size() == 0 )
    {
      continue;
    }

    for( auto& item : section )
    {
      action = dynamic_cast< QAction* >( item );
      menu = dynamic_cast< QMenu* >( item );

      if( action != nullptr )
      {
        m_menu->addAction( action );
      }
      else if( menu != nullptr )
      {
        m_menu->addMenu( menu );
      }
    }

    last_separator = m_menu->addSeparator();
  }

  m_menu->removeAction( last_separator );
}

#include <QtGMenuModel.moc>
