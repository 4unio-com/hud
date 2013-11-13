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

Q_SIGNALS:
  void MenuItemsChanged( QtGMenuModel* model, int position, int removed, int added );

private:
  QtGMenuModel( GMenuModel* model, LinkType link_type, QtGMenuModel* parent, int position );

  static void MenuItemsChangedCallback( GMenuModel* model, gint position, gint removed, gint added,
      gpointer user_data );

  static QtGMenuModel* CreateModel( QtGMenuModel* parent, GMenuModel* model, int position );

  void ConnectCallback();
  void DisconnectCallback();

  void InsertChild( QtGMenuModel* child, int position );
  void ChangeMenuItems( int position, int added, int removed );

  QAction* GetSeparator( QtGMenuModel* for_section );

  void AppendQMenu( std::vector< QMenu* >& menus );

private:
  QtGMenuModel* m_parent;
  QMap< int, QtGMenuModel* > m_children;

  GMenuModel* m_model;
  QMenu* m_menu;
  LinkType m_link_type;
  int m_size;
  gulong m_signal_id;
};

} // namespace qtgmenu

#endif // QTGMENUMODEL_H
