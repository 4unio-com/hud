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

class QtGMenuImporterPrivate : public QObject
{
Q_OBJECT

public:
  QtGMenuImporterPrivate( const QString& service, const QString& path, QtGMenuImporter& parent );
  ~QtGMenuImporterPrivate();

  GMenuModel* GetGMenuModel();
  std::shared_ptr< QMenu > GetQMenu();

private:
  static void MenuItemsChanged( GMenuModel* model, gint position, gint removed, gint added,
      gpointer user_data );

  static void MenuRefresh( GMenuModel* model, gint position, gint removed, gint added,
      gpointer user_data );

  bool RefreshGMenuModel();

  void StopPollingThread();
  void PollingThread();

Q_SIGNALS:
  void MenuRefreshed();

private:
  QtGMenuImporter& m_parent;

  GDBusConnection* m_connection;
  std::string m_service;
  std::string m_path;

  gulong m_sig_handler = 0;

  GMenuModel* m_gmenu_model;
  std::shared_ptr< QMenu > m_qmenu;

  bool m_thread_stop = false;
  bool m_thread_stopped = false;
  std::mutex m_gmenu_model_mutex;
  std::thread m_refresh_thread = std::thread( &QtGMenuImporterPrivate::PollingThread, this );
};

} // namespace qtgmenu

#endif /* QTGMENUEXPORTERPRIVATE_H */
