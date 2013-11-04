#include <QtGMenuImporterPrivate.h>

namespace qtgmenu
{

// this is used to suppress compiler warnings about unused parameters
template< typename... T > void unused( T&&... ) {}

QtGMenuImporterPrivate::QtGMenuImporterPrivate(const QString& service, const QString& path)
    : m_mainloop( g_main_loop_new (NULL, FALSE) ),
      m_connection( g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, NULL) ),
      m_service( service.toStdString() ),
      m_path( path.toStdString() ),
      m_qmenu( new QMenu() )
{
}

QtGMenuImporterPrivate::~QtGMenuImporterPrivate()
{
    g_object_unref(m_connection);
    g_main_loop_unref(m_mainloop);
}

bool QtGMenuImporterPrivate::WaitForItemsChanged( GMenuModel* model, guint timeout )
{
    g_signal_connect(model, "items-changed", G_CALLBACK (ItemsChangedEvent), this);

    g_timeout_add (timeout, ItemsChangedTimeout, this);
    g_main_loop_run (m_mainloop);

    return m_got_items_changed;
}

void QtGMenuImporterPrivate::WaitForItemsChangedReply( bool got_signal )
{
    m_got_items_changed = got_signal;
    g_main_loop_quit(m_mainloop);
}

GMenuModel* QtGMenuImporterPrivate::GetGMenuModel()
{
    GMenuModel* model = G_MENU_MODEL( g_dbus_menu_model_get(
                                          m_connection,
                                          m_service.c_str(),
                                          m_path.c_str()) );

    gint item_count = g_menu_model_get_n_items(model);

    if( item_count == 0 )
    {
        WaitForItemsChanged( model, 500 );
        item_count = g_menu_model_get_n_items(model);
    }

    if( item_count == 0 )
    {
        g_object_unref(model);
        return nullptr;
    }

    return model;
}

std::shared_ptr< QMenu > QtGMenuImporterPrivate::GetQMenu()
{
    return m_qmenu;
}

void QtGMenuImporterPrivate::ItemsChangedEvent(GMenuModel* model, gint position, gint removed, gint added, gpointer user_data)
{
    unused( model, position, removed, added );
    QtGMenuImporterPrivate* importer = (QtGMenuImporterPrivate*)user_data;
    importer->WaitForItemsChangedReply( true );
}

gboolean QtGMenuImporterPrivate::ItemsChangedTimeout (gpointer user_data)
{
    QtGMenuImporterPrivate* importer = (QtGMenuImporterPrivate*)user_data;
    importer->WaitForItemsChangedReply( false );
    return FALSE;
}

} // namespace qtgmenu
