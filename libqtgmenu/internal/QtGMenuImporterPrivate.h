#ifndef QTGMENUEXPORTERPRIVATE_H
#define QTGMENUEXPORTERPRIVATE_H

#include <QtGMenuImporter.h>

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

  void StopRefreshThread();
  void RefreshThread();

private:
  QtGMenuImporter& m_parent;

  GDBusConnection* m_connection;
  std::string m_service;
  std::string m_path;

  GMenuModel* m_gmenu_model;
  std::shared_ptr< QMenu > m_qmenu;

  bool m_refresh_thread_stop = false;
  bool m_refresh_thread_stopped = false;
  std::mutex m_gmenu_model_mutex;
  std::thread m_refresh_thread = std::thread( &QtGMenuImporterPrivate::RefreshThread, this );
};

} // namespace qtgmenu

#endif /* QTGMENUEXPORTERPRIVATE_H */
