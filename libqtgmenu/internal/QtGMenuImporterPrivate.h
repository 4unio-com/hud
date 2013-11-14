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

#ifndef QTGMENUEXPORTERPRIVATE_H
#define QTGMENUEXPORTERPRIVATE_H

#include <QtGMenuImporter.h>
#include <internal/QtGMenuModel.h>

#include <QMenu>
#include <QTimer>
#include <memory>

#undef signals
#include <gio/gio.h>

namespace qtgmenu
{

class QtGMenuImporterPrivate : public QObject
{
  Q_OBJECT

public:
  QtGMenuImporterPrivate( const QString& service, const QString& path, QtGMenuImporter& parent );
  ~QtGMenuImporterPrivate();

  GMenuModel* GetGMenuModel();
  GActionGroup* GetGActionGroup();

  std::vector< QMenu* > GetQMenus();

  void StartPolling( int interval );

private:
  void ClearGMenuModel();
  void ClearGActionGroup();

private Q_SLOTS:
  bool RefreshGMenuModel();
  bool RefreshGActionGroup();

private:
  QtGMenuImporter& m_parent;

  GDBusConnection* m_connection;
  std::string m_service;
  std::string m_path;

  QtGMenuModel* m_gmenu_model = nullptr;
  QTimer m_menu_poll_timer;
  QTimer m_actions_poll_timer;
  QMetaObject::Connection m_items_changed_conn;
  QMetaObject::Connection m_action_added_conn;
  QMetaObject::Connection m_action_removed_conn;
  QMetaObject::Connection m_action_enabled_conn;
  QMetaObject::Connection m_action_state_changed_conn;
};

} // namespace qtgmenu

#endif /* QTGMENUEXPORTERPRIVATE_H */
