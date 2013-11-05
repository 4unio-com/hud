#include <QtGMenuImporterPrivate.h>

namespace qtgmenu
{

// this is used to suppress compiler warnings about unused parameters
template<typename... T> void unused( T&& ... ) {}

QtGMenuImporterPrivate::QtGMenuImporterPrivate(const QString& service, const QString& path)
    : m_mainloop( g_main_loop_new (NULL, FALSE) ),
      m_connection( g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, NULL) ),
      m_service( service.toStdString() ),
      m_path( path.toStdString() ),
      m_gmenu_model( nullptr ),
      m_qmenu( new QMenu() )
{
}

QtGMenuImporterPrivate::~QtGMenuImporterPrivate()
{
    StopMainLoop();
    g_object_unref(m_connection);
    g_main_loop_unref(m_mainloop);

    if ( m_got_gmenu_model )
    {
        g_object_unref(m_gmenu_model);
    }
}

bool QtGMenuImporterPrivate::WaitForItemsChanged( guint timeout )
{
    g_signal_connect(m_gmenu_model, "items-changed", G_CALLBACK (ItemsChangedEvent), this);

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
    RefreshGMenuModel();

    if ( !m_got_gmenu_model )
    {
        return nullptr;
    }

    return m_gmenu_model;
}

std::shared_ptr<QMenu> QtGMenuImporterPrivate::GetQMenu()
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

bool QtGMenuImporterPrivate::RefreshGMenuModel()
{
    std::lock_guard<std::mutex> lock(m_gmenu_model_mutex);

    if ( m_got_gmenu_model )
    {
        return true;
    }

    m_gmenu_model = G_MENU_MODEL( g_dbus_menu_model_get(
                                      m_connection,
                                      m_service.c_str(),
                                      m_path.c_str()) );

    gint item_count = g_menu_model_get_n_items(m_gmenu_model);

    if ( item_count == 0 )
    {
        WaitForItemsChanged( 100 );
        item_count = g_menu_model_get_n_items(m_gmenu_model);
    }

    if ( item_count == 0 )
    {
        g_object_unref(m_gmenu_model);
        return false;
    }

    m_got_gmenu_model = true;
    return true;
}

void QtGMenuImporterPrivate::StopMainLoop()
{
    // set m_main_loop_stop flag (notify ProcessMainLoop to exit)
    m_main_loop_stop = true;

    // wait until ProcessMainLoop exits
    while ( !m_main_loop_stopped )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
    }

    // join with m_main_loop_thread
    m_main_loop_thread.join();
}

void QtGMenuImporterPrivate::ProcessMainLoop()
{
    while ( !m_main_loop_stop )
    {
        RefreshGMenuModel();

        if ( m_got_gmenu_model )
        {
            std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
        }
    }

    // set m_stopped flag (notify StopMainLoop() that the thread has exited)
    m_main_loop_stopped = true;
}

} // namespace qtgmenu
