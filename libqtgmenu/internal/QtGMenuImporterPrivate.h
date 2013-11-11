#ifndef QTGMENUEXPORTERPRIVATE_H
#define QTGMENUEXPORTERPRIVATE_H

#include <QtGMenuImporter.h>

#include <QMenu>
#include <QTimer>
#include <memory>
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

  GMenu* GetGMenu();
  GActionGroup* GetGActionGroup();

  std::shared_ptr< QMenu > GetQMenu();

  void StartPolling( int interval );

private:
  static void MenuItemsChangedCallback( GMenuModel* model, gint position, gint removed, gint added,
      gpointer user_data );

  void ClearGMenuModel();

private Q_SLOTS:
  bool RefreshGMenuModel();
  bool RefreshGActionGroup();

private:
  QtGMenuImporter& m_parent;

  GDBusConnection* m_connection;
  std::string m_service;
  std::string m_path;

  std::shared_ptr< QMenu > m_qmenu;

  GMenuModel* m_gmenu_model;
  gulong m_menu_sig_handler = 0;
  QTimer m_menu_poll_timer;
  std::mutex m_menu_poll_mutex;

  GActionGroup* m_gmenu_actions;
  gulong m_actions_sig_handler = 0;
  QTimer m_actions_poll_timer;
  std::mutex m_actions_poll_mutex;
};

} // namespace qtgmenu

#endif /* QTGMENUEXPORTERPRIVATE_H */
