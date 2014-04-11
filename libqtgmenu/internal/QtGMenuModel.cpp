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
#include <QDebug>

using namespace qtgmenu;

QtGMenuModel::QtGMenuModel( GMenuModel* model )
    : QtGMenuModel( model, LinkType::Root, nullptr, 0 )
{
}

QtGMenuModel::QtGMenuModel( GMenuModel* model, const QString& bus_name, const QString& menu_path, const QMap<QString, QDBusObjectPath>& action_paths )
    : QtGMenuModel( model, LinkType::Root, nullptr, 0 )
{
  m_bus_name = bus_name;
  m_menu_path = menu_path;
  m_action_paths = action_paths;
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
    m_action_paths = m_parent->m_action_paths;

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
    QString qaction_name;
    if( g_menu_model_get_item_attribute( m_parent->m_model, index,
            G_MENU_ATTRIBUTE_ACTION, "s", &action_name ) )
    {
      qaction_name = QString::fromUtf8( action_name );
      g_free( action_name );

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

      if( !qaction_name.isEmpty() )
      {
        QPair<QString, QString> split = QtGMenuUtils::splitPrefixAndName(qaction_name);
        const QString& prefix(split.first);
        if( m_action_paths.contains(prefix) )
        {
          m_ext_menu->menuAction()->setProperty( c_property_actionsPath, m_action_paths[prefix].path() );
        }
      }
    }
  }

  if( m_model )
  {
    m_size = g_menu_model_get_n_items( m_model );
  }

  ChangeMenuItems( 0, m_size, 0 );
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
  auto action_it = m_actions.find( action_name );
  if( action_it != end( m_actions ) )
  {
    action_it->second->setEnabled( enabled );
  }
}

void QtGMenuModel::ActionParameterized( QString action_name, bool parameterized )
{
  auto action_it = m_actions.find( action_name );
  if( action_it != end( m_actions ) )
  {
    action_it->second->setProperty( c_property_isParameterized, parameterized );
  }
}

QtGMenuModel* QtGMenuModel::CreateChild( QtGMenuModel* parent, GMenuModel* model, int index )
{
  QtGMenuModel* new_child = nullptr;
  GMenuLinkIter* link_it = g_menu_model_iterate_item_links( model, index );

  // get the first link, if it exists, create the child accordingly
  if( link_it && g_menu_link_iter_next( link_it ) )
  {
    // if link is a sub menu
    if( strcmp( g_menu_link_iter_get_name( link_it ), G_MENU_LINK_SUBMENU ) == 0 )
    {
      new_child = new QtGMenuModel( g_menu_link_iter_get_value( link_it ), LinkType::SubMenu, parent, index );
    }
    // else if link is a section
    else if( strcmp( g_menu_link_iter_get_name( link_it ), G_MENU_LINK_SECTION ) == 0 )
    {
      new_child = new QtGMenuModel( g_menu_link_iter_get_value( link_it ), LinkType::Section, parent, index );
    }
  }

  g_object_unref( link_it );
  return new_child;
}

void QtGMenuModel::MenuItemsChangedCallback( GMenuModel* model, gint index, gint removed,
    gint added, gpointer user_data )
{
  QtGMenuModel* self = reinterpret_cast< QtGMenuModel* >( user_data );

  if( self->m_model != model )
  {
    qWarning() << "\"items-changed\" signal received from an unrecognised menu model";
    return;
  }

  self->ChangeMenuItems( index, added, removed );
}

void QtGMenuModel::ChangeMenuItems( int index, int added, int removed )
{
  // process removed items first (see “items-changed” on the GMenuModel man page)
  if( removed > 0 )
  {
    // remove QAction from 'index' of our QMenu, 'removed' times
    for( int i = 0; i < removed; ++i )
    {
      if( index < m_menu->actions().size() )
      {
        QAction* at_action = m_menu->actions().at( index );
        ActionRemoved( at_action->property( c_property_actionName ).toString() );
        m_menu->removeAction( at_action );
      }
    }

    // update m_children
    for( int i = index; i < m_size; ++i )
    {
      // remove children from index until ( index + removed )
      if( i < ( index + removed ) )
      {
        delete m_children.take( i );
      }
      // shift children from ( index + removed ) to m_size into the now empty positions
      else if( m_children.contains( i ) )
      {
        m_children.insert( i - removed, m_children.take( i ) );
      }
    }

    // update m_size
    m_size -= removed;
  }

  // now process added items
  if( added > 0 )
  {
    // update m_children
    for( int i = index; i < ( index + added ); ++i )
    {
      // shift 'added' items up from their current index to ( index + added )
      if( m_children.contains( i ) )
      {
        m_children.insert( i + added, m_children.take( i ) );
      }
    }

    // update m_size
    m_size += added;

    // now add a new QAction to our QMenu for each new item
    for( int i = index; i < ( index + added ); ++i )
    {
      QAction* at_action = nullptr;
      if( i < m_menu->actions().size() )
      {
        at_action = m_menu->actions().at( i );
      }

      // try first to create a child model
      QtGMenuModel* model = CreateChild( this, m_model, i );

      // if this is a menu item and not a model
      if( !model )
      {
        QAction* new_action = CreateAction( i );
        ActionAdded( new_action->property( c_property_actionName ).toString(), new_action );
        m_menu->insertAction( at_action, new_action );
      }
      // else if this is a section model
      else if( model->Type() == LinkType::Section )
      {
        m_menu->insertSeparator( at_action );
      }
      // else if this is a sub menu model
      else if( model->Type() == LinkType::SubMenu )
      {
        ActionAdded( model->m_ext_menu->menuAction()->property( c_property_actionName ).toString(),
                     model->m_ext_menu->menuAction() );
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

  // now tell the outside world that items have changed
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
  if( g_menu_model_get_item_attribute( m_model, index, G_MENU_ATTRIBUTE_LABEL, "s", &label ) ) {
    QString qlabel = QString::fromUtf8( label );
    qlabel.replace( '_', '&' );
    g_free( label );

    action->setText( qlabel );
  }

  // action name
  gchar* action_name = NULL;
  if( g_menu_model_get_item_attribute( m_model, index,
	      G_MENU_ATTRIBUTE_ACTION, "s", &action_name ) )
  {
    QString qaction_name = QString::fromUtf8( action_name );
    g_free( action_name );

    action->setProperty( c_property_actionName, qaction_name );
  }

  // is parameterized (set false by default until signal received)
  action->setProperty( c_property_isParameterized, false );

  // dbus paths
  action->setProperty( c_property_busName, m_bus_name );
  action->setProperty( c_property_menuPath, m_menu_path );

  // action icon
  GVariant* icon = g_menu_model_get_item_attribute_value( m_model, index, G_MENU_ATTRIBUTE_ICON,
      G_VARIANT_TYPE_VARIANT );

  if( icon )
  {
    g_variant_unref( icon );
  }

  // action shortcut
  gchar* shortcut = NULL;
  if( g_menu_model_get_item_attribute( m_model, index, "accel", "s", &shortcut ) )
  {
    QString qshortcut = QString::fromUtf8( shortcut );
    g_free( shortcut );

    action->setShortcut( QtGMenuUtils::QStringToQKeySequence( qshortcut ) );
  }

  // action shortcut
  gchar* toolbar_item = NULL;
  if( g_menu_model_get_item_attribute( m_model, index, c_property_hud_toolbar_item, "s", &toolbar_item ) )
  {
    QString qtoolbar_item = QString::fromUtf8( toolbar_item );
    g_free( toolbar_item );

    action->setProperty( c_property_hud_toolbar_item, qtoolbar_item );
  }

  // action keywords
  gchar* keywords = NULL;
  if( g_menu_model_get_item_attribute( m_model, index, c_property_keywords, "s", &keywords ) )
  {
    QVariant qkeywords = QString::fromUtf8( keywords );
    g_free( keywords );

    action->setProperty( c_property_keywords, qkeywords );
  }

  // action trigger
  connect( action, SIGNAL( triggered( bool ) ), this, SLOT( ActionTriggered( bool ) ) );

  return action;
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

void QtGMenuModel::ActionAdded( const QString& name, QAction* action )
{
  // add action to top menu's m_actions
  if( m_parent )
  {
    m_parent->ActionAdded( name, action );
  }

  m_actions[name] = action;
}

void QtGMenuModel::ActionRemoved( const QString& name )
{
  // remove action from top menu's m_actions
  if( m_parent )
  {
    m_parent->ActionRemoved( name );
  }

  m_actions.erase( name );
}
