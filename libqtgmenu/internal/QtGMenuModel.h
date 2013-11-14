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

#ifndef QTGMENUMODEL_H
#define QTGMENUMODEL_H

#include <QObject>
#include <QMap>
#include <QMenu>

#include <memory>
#include <deque>

#undef signals
#include <gio/gio.h>

namespace qtgmenu
{

class QtGMenuModel : public QObject
{
  Q_OBJECT

public:
  enum class LinkType
  {
    Root, Section, SubMenu
  };

  QtGMenuModel( GMenuModel* model );
  ~QtGMenuModel();

  GMenuModel* Model() const;
  LinkType Type() const;

  int Size() const;

  QtGMenuModel* Parent() const;
  QtGMenuModel* Child( int position ) const;

  std::vector< QMenu* > GetQMenus();

  GActionGroup* ActionGroup() const;
  void SetActionGroup( GActionGroup* action_group );
  QString ActionAt( int position );
  int ActionsCount();

Q_SIGNALS:
  void MenuItemsChanged( QtGMenuModel* model, int position, int removed, int added );

  void ActionAdded( QString action_name );
  void ActionRemoved( QString action_name );
  void ActionEnabled( QString action_name, bool enabled );
  void ActionStateChanged( QString action_name, QVariant value );

private:
  QtGMenuModel( GMenuModel* model, LinkType link_type, QtGMenuModel* parent, int position );

  static void MenuItemsChangedCallback( GMenuModel* model, gint position, gint removed, gint added,
      gpointer user_data );
  static void ActionAddedCallback( GActionGroup* action_group, gchar* action_name,
      gpointer user_data );
  static void ActionRemovedCallback( GActionGroup* action_group, gchar* action_name,
      gpointer user_data );
  static void ActionEnabledCallback( GActionGroup* action_group, gchar* action_name,
      gboolean enabled, gpointer user_data );
  static void ActionStateChangedCallback( GActionGroup* action_group, gchar* action_name,
      GVariant* value, gpointer user_data );

  void ChangeMenuItems( int position, int added, int removed );

  static QtGMenuModel* CreateModel( QtGMenuModel* parent, GMenuModel* model, int position );

  void ConnectMenuCallback();
  void DisconnectMenuCallback();

  void ConnectActionCallbacks();
  void DisconnectActionCallbacks();

  void InsertChild( QtGMenuModel* child, int position );
  int ChildPosition( QtGMenuModel* child );

  QAction* CreateAction( int position );

  void AppendQMenu( std::vector< QMenu* >& menus );
  void RefreshQMenu();

private:
  QtGMenuModel* m_parent = nullptr;
  QMap< int, QtGMenuModel* > m_children;

  GMenuModel* m_model = nullptr;
  gulong m_items_changed_handler = 0;

  GActionGroup* m_action_group = nullptr;
  gulong m_action_added_handler = 0;
  gulong m_action_removed_handler = 0;
  gulong m_action_enabled_handler = 0;
  gulong m_action_state_changed_handler = 0;

  LinkType m_link_type;
  int m_size = 0;

  QMenu* m_menu = new QMenu();
  std::deque< std::deque< QObject* > > m_sections;
};

} // namespace qtgmenu

#endif // QTGMENUMODEL_H
