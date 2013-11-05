#ifndef QTGMENUEXPORTERPRIVATE_H
#define QTGMENUEXPORTERPRIVATE_H

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
    QtGMenuImporterPrivate(const QString& service, const QString& path);
    ~QtGMenuImporterPrivate();

    bool WaitForItemsChanged(guint timeout );
    void WaitForItemsChangedReply( bool got_signal );

    GMenuModel* GetGMenuModel();
    std::shared_ptr<QMenu> GetQMenu();

private:
    static void ItemsChangedEvent(GMenuModel* model, gint position, gint removed, gint added, gpointer user_data);
    static gboolean ItemsChangedTimeout (gpointer user_data);

    bool RefreshGMenuModel();

    void StopMainLoop();
    void ProcessMainLoop();

private:
    GMainLoop* m_mainloop;
    GDBusConnection* m_connection;
    std::string m_service;
    std::string m_path;

    GMenuModel* m_gmenu_model;
    std::shared_ptr<QMenu> m_qmenu;

    std::mutex m_gmenu_model_mutex;
    std::mutex m_gmenu_signal_mutex;
    bool m_got_items_changed = false;
    bool m_got_gmenu_model = false;

    bool m_main_loop_stop = false;
    bool m_main_loop_stopped = false;
    std::thread m_main_loop_thread = std::thread( &QtGMenuImporterPrivate::ProcessMainLoop, this );
};

} // namespace qtgmenu

#endif /* QTGMENUEXPORTERPRIVATE_H */
