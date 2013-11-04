#ifndef QTGMENUEXPORTERPRIVATE_H
#define QTGMENUEXPORTERPRIVATE_H

#include <QMenu>
#include <memory>

#undef signals
#include <gio/gio.h>

namespace qtgmenu
{

class QtGMenuImporterPrivate
{
public:
    QtGMenuImporterPrivate(const QString& service, const QString& path);
    ~QtGMenuImporterPrivate();

    bool WaitForItemsChanged( GMenuModel* model, guint timeout );
    void WaitForItemsChangedReply( bool got_signal );

    GMenuModel* GetGMenuModel();
    std::shared_ptr< QMenu > GetQMenu();

private:
    static void ItemsChangedEvent(GMenuModel* model, gint position, gint removed, gint added, gpointer user_data);
    static gboolean ItemsChangedTimeout (gpointer user_data);

private:
    GMainLoop* m_mainloop;
    GDBusConnection* m_connection;
    std::string m_service;
    std::string m_path;

    std::shared_ptr< QMenu > m_qmenu;

    bool m_got_items_changed;
};

} // namespace qtgmenu

#endif /* QTGMENUEXPORTERPRIVATE_H */
