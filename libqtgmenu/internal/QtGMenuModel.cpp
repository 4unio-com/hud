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

QtGMenuModel::~QtGMenuModel()
{
  DisconnectCallback();

  for( auto child : m_children )
  {
    delete child;
  }
  m_children.clear();

  delete m_menu;

  if( m_model )
  {
    g_object_unref( m_model );
  }
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

QtGMenuModel::QtGMenuModel( GMenuModel* model, LinkType link_type, QtGMenuModel* parent,
    int position )
    : m_parent( parent ),
      m_model( model ),
      m_menu( new QMenu() ),
      m_link_type( link_type ),
      m_signal_id( 0 )
{
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
    m_size = g_menu_model_get_n_items( model );
  }

  for( int i = 0; i < m_size; ++i )
  {
    QtGMenuModel::CreateModel( this, model, i );
  }

  ConnectCallback();
}

void QtGMenuModel::MenuItemsChangedCallback( GMenuModel* model, gint position, gint removed,
    gint added, gpointer user_data )
{
  QtGMenuModel* self = reinterpret_cast< QtGMenuModel* >( user_data );
  self->ChangeMenuItems( position, added, removed );
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

void QtGMenuModel::ConnectCallback()
{
  if( m_model && m_signal_id == 0 )
  {
    m_signal_id = g_signal_connect( m_model, "items-changed",
        G_CALLBACK( QtGMenuModel::MenuItemsChangedCallback ), this );
  }
}

void QtGMenuModel::DisconnectCallback()
{
  if( m_signal_id != 0 )
  {
    g_signal_handler_disconnect( m_model, m_signal_id );
  }
}

void QtGMenuModel::InsertChild( QtGMenuModel* child, int position )
{
  if( m_children.contains( position ) )
  {
    return;
  }

  child->m_parent = this;
  m_children.insert( position, child );

  connect( child, SIGNAL( MenuItemsChanged(QtGMenuModel*,int,int,int)), this,
      SIGNAL( MenuItemsChanged(QtGMenuModel*,int,int,int)) );
}

void QtGMenuModel::ChangeMenuItems( int position, int added, int removed )
{
  if( added > 0 )
  {
    for( int i = ( m_size - 1 + added ), iMin = position; i >= iMin; --i )
    {
      if( m_children.contains( i ) )
      {
        m_children.insert( i + added, m_children.take( i ) );
      }
    }

    m_size += added;

    QMenu* changed_menu = m_menu;
    QAction* before_separator = nullptr;
    QActionGroup* new_actions = new QActionGroup( this );

    if( m_link_type == LinkType::Section && m_parent )
    {
      changed_menu = m_parent->m_menu;
      before_separator = m_parent->GetSeparator( this );
    }

    if( changed_menu->actions().size() != 0 )
    {
      new_actions->addAction( "" );
      new_actions->actions().at( 0 )->setSeparator( true );
    }

    for( int i = position; i < ( position + added ); ++i )
    {
      if( !QtGMenuModel::CreateModel( this, m_model, i ) )
      {
        GVariant* label = g_menu_model_get_item_attribute_value( m_model, i, G_MENU_ATTRIBUTE_LABEL,
            G_VARIANT_TYPE_STRING );
        QString qlabel = QtGMenuUtils::GVariantToQVariant( label ).toString();
        qlabel.replace( '_', '&' );
        g_variant_unref( label );

        new_actions->addAction( qlabel );
      }
    }

    changed_menu->insertActions( before_separator, new_actions->actions() );
  }

  if( removed > 0 )
  {
    int removedEnd = position + removed;
    for( int i = position, iMax = m_size; i < iMax; ++i )
    {
      if( i <= removedEnd )
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

  emit MenuItemsChanged( this, position, removed, added );
}

QAction* QtGMenuModel::GetSeparator( QtGMenuModel* for_section )
{
  int separator_no = 0;

  for( int i = 1; i < m_children.size() - 1; ++i )
  {
    if( for_section == m_children[i] )
    {
      separator_no = i;
      break;
    }
  }

  if( separator_no == 0 )
  {
    return nullptr;
  }

  for( auto action : m_menu->actions() )
  {
    if( action->isSeparator() )
    {
      --separator_no;
    }
    if( separator_no == 0 )
    {
      return action;
    }
  }

  return nullptr;
}

void QtGMenuModel::AppendQMenu( std::vector< QMenu* >& menus )
{
  menus.push_back( m_menu );

  if( m_link_type == LinkType::SubMenu )
  {
    // only append one menu containing nested menus
  }
  else
  {
    for( auto& child : m_children )
    {
      child->AppendQMenu( menus );
    }
  }
}

#include <QtGMenuModel.moc>
