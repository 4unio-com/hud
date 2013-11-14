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
  QtGMenuModel* Child( int index ) const;

  std::vector< QMenu* > GetQMenus();

Q_SIGNALS:
  void MenuItemsChanged( QtGMenuModel* model, int index, int removed, int added );

private:
  QtGMenuModel( GMenuModel* model, LinkType link_type, QtGMenuModel* parent, int index );

  static QtGMenuModel* CreateModel( QtGMenuModel* parent, GMenuModel* model, int index );

  static void MenuItemsChangedCallback( GMenuModel* model, gint index, gint removed, gint added,
      gpointer user_data );

  void ChangeMenuItems( int index, int added, int removed );

  void ConnectCallback();
  void DisconnectCallback();

  void InsertChild( QtGMenuModel* child, int index );
  int ChildIndex( QtGMenuModel* child );

  QAction* CreateAction( int index );

  void AppendQMenu( std::vector< QMenu* >& menus );
  void RefreshQMenu();

private:
  QtGMenuModel* m_parent = nullptr;
  QMap< int, QtGMenuModel* > m_children;

  GMenuModel* m_model = nullptr;
  gulong m_items_changed_handler = 0;

  LinkType m_link_type;
  int m_size = 0;

  QMenu* m_menu = new QMenu();
  std::deque< std::deque< QObject* > > m_sections;
};

} // namespace qtgmenu

#endif // QTGMENUMODEL_H
