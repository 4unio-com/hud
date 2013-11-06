#ifndef QTGMENUEXPORTERPRIVATE_H
#define QTGMENUEXPORTERPRIVATE_H

#include <QtGMenuImporter.h>
#include <QtGMenuConverter.h>

#include <QMenu>
#include <memory>
#include <thread>
#include <mutex>

#undef signals
#include <gio/gio.h>

namespace qtgmenu
{

class QtGMenuImporterPrivate
{
public:
  QtGMenuImporterPrivate( const QString& service, const QString& path, QtGMenuImporter& parent );
  ~QtGMenuImporterPrivate();

  GMenuModel* GetGMenuModel();
  std::shared_ptr< QMenu > GetQMenu();

private:
  static void MenuItemsChanged( GMenuModel* model, gint position, gint removed, gint added,
      gpointer user_data );

  bool RefreshGMenuModel();
  bool RefreshQMenu();

private:
  QtGMenuImporter& m_parent;
  QtGMenuConverter m_converter;

  GDBusConnection* m_connection;
  std::string m_service;
  std::string m_path;

  GMenuModel* m_gmenu_model;
  std::shared_ptr< QMenu > m_qmenu;
};

} // namespace qtgmenu

#endif /* QTGMENUEXPORTERPRIVATE_H */
