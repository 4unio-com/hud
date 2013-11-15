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

QtGMenuModel::QtGMenuModel( GMenuModel* model, LinkType link_type, QtGMenuModel* parent, int index )
    : m_parent( parent ),
      m_model( model ),
      m_link_type( link_type )
{
  if( m_parent )
  {
    m_parent->InsertChild( this, index );

    GVariant* label = g_menu_model_get_item_attribute_value( m_parent->m_model, index,
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

  ConnectCallback();
}

QtGMenuModel::~QtGMenuModel()
{
  if( m_model )
  {
    MenuItemsChangedCallback( m_model, 0, Size(), 0, this );
    DisconnectCallback();
    g_object_unref( m_model );
  }

  for( auto child : m_children )
  {
    delete child;
  }
  m_children.clear();

  delete m_menu;
  m_menu = nullptr;
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

QtGMenuModel* QtGMenuModel::Child( int index ) const
{
  if( m_children.contains( index ) )
  {
    return m_children.value( index );
  }

  return nullptr;
}

std::shared_ptr< QMenu > QtGMenuModel::GetQMenu()
{
  auto top_menu = std::make_shared< QMenu >();

  AppendQMenu( top_menu );

  return top_menu;
}

void QtGMenuModel::ActionTriggered( bool checked )
{
  QAction* action = dynamic_cast< QAction* >( QObject::sender() );
  emit ActionTriggered( action->objectName(), checked );
}

QtGMenuModel* QtGMenuModel::CreateModel( QtGMenuModel* parent, GMenuModel* model, int index )
{
  LinkType linkType( LinkType::SubMenu );
  GMenuModel* link = g_menu_model_get_item_link( model, index, G_MENU_LINK_SUBMENU );

  if( !link )
  {
    linkType = LinkType::Section;
    link = g_menu_model_get_item_link( model, index, G_MENU_LINK_SECTION );
  }

  if( link )
  {
    return new QtGMenuModel( link, linkType, parent, index );
  }

  return nullptr;
}

void QtGMenuModel::MenuItemsChangedCallback( GMenuModel* model, gint index, gint removed,
    gint added, gpointer user_data )
{
  QtGMenuModel* self = reinterpret_cast< QtGMenuModel* >( user_data );
  self->ChangeMenuItems( index, added, removed );
}

void QtGMenuModel::ChangeMenuItems( int index, int added, int removed )
{
  QtGMenuModel* model_effected = this;
  QAction* at_action = nullptr;
  if( m_link_type == LinkType::Section && m_parent )
  {
    model_effected = m_parent;
  }

  if( removed > 0 )
  {
    for( int i = 0; i < removed; ++i )
    {
      if( index < m_menu->actions().size() )
      {
        at_action = m_menu->actions().at( index );
      }

      m_menu->removeAction( at_action );
    }

    for( int i = index; i < m_size; ++i )
    {
      if( i <= ( index + removed ) )
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
    for( int i = ( m_size + added ) - 1; i >= index; --i )
    {
      if( m_children.contains( i ) )
      {
        m_children.insert( i + added, m_children.take( i ) );
      }
    }

    m_size += added;

    for( int i = index; i < ( index + added ); ++i )
    {
      QtGMenuModel* model = CreateModel( this, m_model, i );
      if( !model )
      {
        if( index + i < m_menu->actions().size() )
        {
          at_action = m_menu->actions().at( index + i );
        }

        m_menu->insertAction( at_action, CreateAction( i ) );
      }
      else if( model->Type() == LinkType::SubMenu )
      {
        if( index + i < m_menu->actions().size() )
        {
          at_action = m_menu->actions().at( index + i );
        }

        m_menu->insertMenu( at_action, model->m_menu );
      }
    }
  }

  model_effected->RefreshQMenu();
  emit MenuItemsChanged( this, index, removed, added );
}

void QtGMenuModel::ConnectCallback()
{
  if( m_model && m_items_changed_handler == 0 )
  {
    m_items_changed_handler = g_signal_connect( m_model, "items-changed",
        G_CALLBACK( MenuItemsChangedCallback ), this );
  }
}

void QtGMenuModel::DisconnectCallback()
{
  if( m_model && m_items_changed_handler != 0 )
  {
    g_signal_handler_disconnect( m_model, m_items_changed_handler );
  }

  m_items_changed_handler = 0;
}

void QtGMenuModel::InsertChild( QtGMenuModel* child, int index )
{
  if( m_children.contains( index ) )
  {
    return;
  }

  child->m_parent = this;
  m_children.insert( index, child );

  connect( child, SIGNAL( MenuItemsChanged( QtGMenuModel*, int, int, int ) ), this,
      SIGNAL( MenuItemsChanged( QtGMenuModel*, int, int, int ) ) );

  connect( child, SIGNAL( ActionTriggered( QString, bool ) ), this,
      SIGNAL( ActionTriggered( QString, bool ) ) );
}

int QtGMenuModel::ChildIndex( QtGMenuModel* child )
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

QAction* QtGMenuModel::CreateAction( int index )
{
  QAction* action = new QAction( this );

  GVariant* label = g_menu_model_get_item_attribute_value( m_model, index, G_MENU_ATTRIBUTE_LABEL,
      G_VARIANT_TYPE_STRING );

  QString qlabel = QtGMenuUtils::GVariantToQVariant( label ).toString();
  qlabel.replace( '_', '&' );
  g_variant_unref( label );

  GVariant* action_name = g_menu_model_get_item_attribute_value( m_model, index,
      G_MENU_ATTRIBUTE_ACTION, G_VARIANT_TYPE_STRING );

  QString qaction_name = QtGMenuUtils::GVariantToQVariant( action_name ).toString();
  g_variant_unref( action_name );

  GVariant* icon = g_menu_model_get_item_attribute_value( m_model, index, G_MENU_ATTRIBUTE_ICON,
      G_VARIANT_TYPE_VARIANT );

  connect( action, SIGNAL( triggered( bool ) ), this, SLOT( ActionTriggered( bool ) ) );

  action->setEnabled( true );
  action->setObjectName( qaction_name );

  action->setText( qlabel );
  return action;
}

void QtGMenuModel::AppendQMenu( std::shared_ptr< QMenu > top_menu )
{
  top_menu->addAction( m_menu->menuAction() );

  if( m_link_type != LinkType::SubMenu )
  {
    for( auto& child : m_children )
    {
      child->AppendQMenu( top_menu );
    }
  }
}

void QtGMenuModel::RefreshQMenu()
{
  // loop through children
  // if child is action, add action
  // if child section, add its menu items to this qmenu
  // if child is submenu, add menu

  QAction* last_separator = nullptr;

  m_menu->clear();

  for( int i = 0; i < m_size; ++i )
  {
    QtGMenuModel* child = Child( i );

    if( !child )
    {
      ///!
    }
    else if( child->Type() == LinkType::Section )
    {
      for( QAction* action : child->m_menu->actions() )
      {
        m_menu->addAction( action );
      }
    }
    else if( child->Type() == LinkType::SubMenu )
    {
      m_menu->addMenu( child->m_menu );
    }

    last_separator = m_menu->addSeparator();
  }

  m_menu->removeAction( last_separator );
}

#include <QtGMenuModel.moc>
