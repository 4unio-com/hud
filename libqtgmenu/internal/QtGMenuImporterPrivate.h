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

  static void ActionAddedCallback( GActionGroup *action_group, gchar *action_name,
      gpointer user_data );

  static void ActionEnabledCallback( GActionGroup *action_group, gchar *action_name,
      gboolean enabled, gpointer user_data );

  static void ActionRemovedCallback( GActionGroup *action_group, gchar *action_name,
      gpointer user_data );

  static void ActionStateChangedCallback( GActionGroup *action_group, gchar *action_name,
      GVariant *value, gpointer user_data );

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

  std::shared_ptr< QMenu > m_qmenu;

  GMenuModel* m_gmenu_model;
  gulong m_menu_items_changed_handler = 0;
  QTimer m_menu_poll_timer;
  std::mutex m_menu_poll_mutex;

  GActionGroup* m_gaction_group;
  gulong m_action_added_handler = 0;
  gulong m_action_enabled_handler = 0;
  gulong m_action_removed_handler = 0;
  gulong m_action_state_changed_handler = 0;
  QTimer m_actions_poll_timer;
  std::mutex m_actions_poll_mutex;
};

} // namespace qtgmenu

#endif /* QTGMENUEXPORTERPRIVATE_H */
