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

#ifndef QTGMENUCONVERTER_H
#define QTGMENUCONVERTER_H

#include <memory>
#include <QMenu>

class _GMenuModel;
typedef _GMenuModel GMenuModel;

namespace qtgmenu
{

class QtGMenuConverterPrivate;

class QtGMenuConverter final
{
public:
  QtGMenuConverter();
  ~QtGMenuConverter();

  static std::vector< std::shared_ptr< QMenu > > ToQMenus( GMenuModel* model );

private:
  Q_DISABLE_COPY(QtGMenuConverter)
  std::unique_ptr< QtGMenuConverterPrivate > d;
};

} // namespace qtgmenu

#endif // QTGMENUCONVERTER_H
