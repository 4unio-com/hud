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

QtGMenuModel::QtGMenuModel( GMenuModel* model, const QString& bus_name, const QString& menu_path,
    const QString& actions_path )
    : QtGMenuModel( model, LinkType::Root, nullptr, 0 )
{
  m_bus_name = bus_name;
  m_menu_path = menu_path;
  m_actions_path = actions_path;
}

QtGMenuModel::QtGMenuModel( GMenuModel* model, LinkType link_type, QtGMenuModel* parent, int index )
    : m_parent( parent ),
      m_model( model ),
      m_link_type( link_type )
{
  ConnectCallback();

  if( m_parent )
  {
    m_parent->InsertChild( this, index );

    m_bus_name = m_parent->m_bus_name;
    m_menu_path = m_parent->m_menu_path;
    m_actions_path = m_parent->m_actions_path;

    gchar* label = NULL;
    if( g_menu_model_get_item_attribute( m_parent->m_model, index,
            G_MENU_ATTRIBUTE_LABEL, "s", &label ) )
    {
      QString qlabel = QString::fromUtf8( label );
      qlabel.replace( '_', '&' );
      g_free( label );

      m_ext_menu->setTitle( qlabel );
    }

    gchar* action_name = NULL;
    if( g_menu_model_get_item_attribute( m_parent->m_model, index,
            G_MENU_ATTRIBUTE_ACTION, "s", &action_name ) )
    {
      QString qaction_name = QString::fromUtf8( action_name );
      g_free( action_name );

      int name_length = qaction_name.size() - qaction_name.indexOf( '.' ) - 1;
      qaction_name = qaction_name.right( name_length );

      m_ext_menu->menuAction()->setProperty( c_property_actionName, qaction_name );
    }

    // if this model has a "commitLabel" property, it is a libhud parameterized action
    gchar* commit_label = NULL;
    if( g_menu_model_get_item_attribute( m_parent->m_model, index,
            "commitLabel", "s", &commit_label ) )
    {
      g_free( commit_label );

      // is parameterized
      m_ext_menu->menuAction()->setProperty( c_property_isParameterized, true );

      // dbus paths
      m_ext_menu->menuAction()->setProperty( c_property_busName, m_bus_name );
      m_ext_menu->menuAction()->setProperty( c_property_menuPath, m_menu_path );
      m_ext_menu->menuAction()->setProperty( c_property_actionsPath, m_actions_path );
    }
  }

  if( m_model )
  {
    m_size = g_menu_model_get_n_items( m_model );
  }

  for( int i = 0; i < m_size; ++i )
  {
    CreateChild( this, m_model, i );
  }
}

QtGMenuModel::~QtGMenuModel()
{
  if( m_model )
  {
    if( m_size > 0 )
    {
      MenuItemsChangedCallback( m_model, 0, m_size, 0, this );
    }
    DisconnectCallback();
    g_object_unref( m_model );
  }

  for( auto child : m_children )
  {
    delete child;
  }
  m_children.clear();

  delete m_menu;
  delete m_ext_menu;
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
  emit ActionTriggered( action->property( c_property_actionName ).toString(), checked );
}

void QtGMenuModel::ActionEnabled( QString action_name, bool enabled )
{
  QAction* action = FindAction( action_name );
  if( action )
  {
    action->setEnabled( enabled );
  }
}

void QtGMenuModel::ActionParameterized( QString action_name, bool parameterized )
{
  QAction* action = FindAction( action_name );
  if( action )
  {
    action->setProperty( c_property_isParameterized, parameterized );
  }
}

QtGMenuModel* QtGMenuModel::CreateChild( QtGMenuModel* parent, GMenuModel* model, int index )
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
  if( removed > 0 )
  {
    for( int i = 0; i < removed; ++i )
    {
      if( index < m_menu->actions().size() )
      {
        QAction* at_action = m_menu->actions().at( index );
        m_menu->removeAction( at_action );
      }
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
      QAction* at_action = nullptr;
      if( i < m_menu->actions().size() )
      {
        at_action = m_menu->actions().at( i );
      }

      QtGMenuModel* model = CreateChild( this, m_model, i );

      if( !model )
      {
        m_menu->insertAction( at_action, CreateAction( i ) );
      }
      else if( model->Type() == LinkType::Section )
      {
        m_menu->insertSeparator( at_action );
      }
      else if( model->Type() == LinkType::SubMenu )
      {
        m_menu->insertMenu( at_action, model->m_ext_menu );
      }
    }
  }

  // update external menu
  UpdateExtQMenu();
  if( m_link_type == LinkType::Section && m_parent )
  {
    m_parent->UpdateExtQMenu();
  }

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
  // action label
  QAction* action = new QAction( this );

  gchar* label = NULL;
  g_menu_model_get_item_attribute( m_model, index, G_MENU_ATTRIBUTE_LABEL,
      "s", &label );

  QString qlabel = QString::fromUtf8( label );
  qlabel.replace( '_', '&' );
  g_free( label );

  action->setText( qlabel );

  // action name
  gchar* action_name = NULL;
  if( g_menu_model_get_item_attribute( m_model, index,
	      G_MENU_ATTRIBUTE_ACTION, "s", &action_name ) )
  {
    QString qaction_name = QString::fromUtf8( action_name );
    g_free( action_name );

    int name_length = qaction_name.size() - qaction_name.indexOf( '.' ) - 1;
    qaction_name = qaction_name.right( name_length );

    action->setProperty( c_property_actionName, qaction_name );
  }

  // is parameterized (set false by default until signal received)
  action->setProperty( c_property_isParameterized, false );

  // dbus paths
  action->setProperty( c_property_busName, m_bus_name );
  action->setProperty( c_property_menuPath, m_menu_path );
  action->setProperty( c_property_actionsPath, m_actions_path );

  // action icon
  GVariant* icon = g_menu_model_get_item_attribute_value( m_model, index, G_MENU_ATTRIBUTE_ICON,
      G_VARIANT_TYPE_VARIANT );

  if( icon )
  {
    g_variant_unref( icon );
  }

  // action shortcut
  GMenuItem* menu_item = g_menu_item_new_from_model( m_model, index );
  gchar* shortcut = NULL;

  if( g_menu_item_get_attribute( menu_item, "accel", "s", &shortcut ) )
  {
    QString qshortcut = QString::fromUtf8( shortcut );
    g_free( shortcut );

    action->setShortcut( QtGMenuUtils::QStringToQKeySequence( qshortcut ) );
  }

  // action shortcut
  gchar* toolbar_item = NULL;
  if( g_menu_item_get_attribute( menu_item, "hud-toolbar-item", "s", &toolbar_item ) )
  {
    QString qtoolbar_item = QString::fromUtf8( toolbar_item );
    g_free( toolbar_item );

    action->setProperty( "hud-toolbar-item", qtoolbar_item );
  }

  // action keywords
  gchar* keywords = NULL;
  if( g_menu_item_get_attribute( menu_item, "keywords", "s", keywords ) )
  {
    QVariant qkeywords = QString::fromUtf8( keywords );
    g_free( keywords );

    action->setProperty( c_property_keywords, qkeywords );
  }

  g_object_unref( menu_item );

  // action trigger
  connect( action, SIGNAL( triggered( bool ) ), this, SLOT( ActionTriggered( bool ) ) );

  return action;
}

QAction* QtGMenuModel::FindAction( QString name )
{
  if( m_ext_menu->menuAction()->property( c_property_actionName ) == name )
  {
    return m_ext_menu->menuAction();
  }

  for( QAction* action : m_menu->actions() )
  {
    if( action->property( c_property_actionName ) == name )
    {
      return action;
    }
  }

  for( QtGMenuModel* child : m_children )
  {
    QAction* action = child->FindAction( name );
    if( action )
    {
      return action;
    }
  }

  return nullptr;
}

void QtGMenuModel::AppendQMenu( std::shared_ptr< QMenu > top_menu )
{
  if( m_link_type == LinkType::Root )
  {
    for( QAction* action : m_ext_menu->actions() )
    {
      if( !action->menu() )
      {
        top_menu->addAction( action );
      }
    }
  }
  else if( m_link_type == LinkType::SubMenu )
  {
    top_menu->addAction( m_ext_menu->menuAction() );
  }

  if( m_link_type != LinkType::SubMenu )
  {
    for( auto& child : m_children )
    {
      child->AppendQMenu( top_menu );
    }
  }
}

void QtGMenuModel::UpdateExtQMenu()
{
  m_ext_menu->clear();

  for( int i = 0; i < m_menu->actions().size(); ++i )
  {
    QAction* action = m_menu->actions().at( i );

    if( action->isSeparator() )
    {
      QtGMenuModel* child = Child( i );
      if( !child || child->Type() != LinkType::Section )
      {
        continue;
      }
      QMenu* section = child->m_ext_menu;

      for( QAction* sub_action : section->actions() )
      {
        m_ext_menu->addAction( sub_action );
      }
      m_ext_menu->addSeparator();
    }
    else
    {
      m_ext_menu->addAction( action );
    }
  }

  if( m_ext_menu->actions().size() > 0 )
  {
    QAction* last_action = m_ext_menu->actions().last();
    if( last_action && last_action->isSeparator() )
    {
      m_ext_menu->removeAction( last_action );
    }
  }
}

#include <QtGMenuModel.moc>
