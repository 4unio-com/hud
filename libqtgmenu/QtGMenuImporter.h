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

#ifndef QTGMENUIMPORTER_H
#define QTGMENUIMPORTER_H

#include <QObject>
#include <memory>

class QMenu;

class _GMenuModel;
typedef _GMenuModel GMenuModel;

class _GActionGroup;
typedef _GActionGroup GActionGroup;

namespace qtgmenu
{

class QtGMenuImporterPrivate;

class QtGMenuImporter final : public QObject
{
Q_OBJECT

public:
  QtGMenuImporter( const QString& service, const QString& path, QObject* parent = 0 );
  QtGMenuImporter( const QString& service, const QString& menu_path, const QString& actions_path, QObject* parent = 0 );
  ~QtGMenuImporter();

  GMenuModel* GetGMenuModel() const;
  GActionGroup* GetGActionGroup() const;

  std::shared_ptr< QMenu > GetQMenu() const;

  void ForceRefresh();

Q_SIGNALS:
  void MenuItemsChanged();
  void MenuAppeared();
  void MenuDisappeared();

  void ActionAdded( QString action_name );
  void ActionRemoved( QString action_name );
  void ActionEnabled( QString action_name, bool enabled );
  void ActionStateChanged( QString action_name, QVariant value );
  void ActionsAppeared();
  void ActionsDisappeared();

private:
  Q_DISABLE_COPY (QtGMenuImporter)
  std::unique_ptr< QtGMenuImporterPrivate > d;
};

} // namespace qtgmenu

#endif // QTGMENUIMPORTER_H
