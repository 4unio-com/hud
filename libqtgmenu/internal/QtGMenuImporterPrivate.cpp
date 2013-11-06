#include <QtGMenuImporterPrivate.h>
#include <QEventLoop>
#include <QTimer>

namespace qtgmenu
{

// this is used to suppress compiler warnings about unused parameters
template< typename ... T > void unused( T&& ... )
{
}

QtGMenuImporterPrivate::QtGMenuImporterPrivate( const QString& service, const QString& path,
    QtGMenuImporter& parent )
    : m_parent( parent ),
      m_connection( g_bus_get_sync( G_BUS_TYPE_SESSION, NULL, NULL ) ),
      m_service( service.toStdString() ),
      m_path( path.toStdString() ),
      m_gmenu_model( nullptr ),
      m_qmenu( new QMenu() )
{
}

QtGMenuImporterPrivate::~QtGMenuImporterPrivate()
{
  StopRefreshThread();

  g_object_unref( m_connection );

  if( m_gmenu_model != nullptr )
  {
    g_object_unref( m_gmenu_model );
  }
}

GMenuModel* QtGMenuImporterPrivate::GetGMenuModel()
{
  RefreshGMenuModel();

  return m_gmenu_model;
}

std::shared_ptr< QMenu > QtGMenuImporterPrivate::GetQMenu()
{
  return m_qmenu;
}

void QtGMenuImporterPrivate::MenuItemsChanged( GMenuModel* model, gint position, gint removed,
    gint added, gpointer user_data )
{
  unused( model, position, removed, added );
  QtGMenuImporter* importer = ( QtGMenuImporter* ) user_data;
  emit importer->MenuItemsChanged();
}

bool QtGMenuImporterPrivate::RefreshGMenuModel()
{
  std::lock_guard < std::mutex > lock( m_gmenu_model_mutex );

  if( m_gmenu_model != nullptr )
  {
    return true;
  }

  m_gmenu_model =
      G_MENU_MODEL( g_dbus_menu_model_get( m_connection, m_service.c_str(), m_path.c_str()) );

  gulong sig_handler =
      g_signal_connect(m_gmenu_model, "items-changed", G_CALLBACK (MenuItemsChanged), &m_parent);

  gint item_count = g_menu_model_get_n_items( m_gmenu_model );

  if( item_count == 0 )
  {
    QEventLoop loop;
    QTimer timeout;
    loop.connect( &m_parent, SIGNAL(MenuItemsChanged()), SLOT(quit()) );
    timeout.singleShot( 100, &loop, SLOT(quit()) );
    loop.exec(); // blocks until MenuItemsChanged fires or timeout reached

    item_count = g_menu_model_get_n_items( m_gmenu_model );
  }

  if( item_count == 0 )
  {
    g_signal_handler_disconnect( m_gmenu_model, sig_handler );
    g_object_unref( m_gmenu_model );
    m_gmenu_model = nullptr;
    return false;
  }

  return true;
}

void QtGMenuImporterPrivate::StopRefreshThread()
{
  // set m_refresh_thread_stop flag (notify RefreshThread to exit)
  m_refresh_thread_stop = true;

  // wait until RefreshThread exits
  while( !m_refresh_thread_stopped )
  {
    std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
  }

  // join with m_refresh_thread
  m_refresh_thread.join();
}

void QtGMenuImporterPrivate::RefreshThread()
{
  while( m_gmenu_model == nullptr && !m_refresh_thread_stop )
  {
    RefreshGMenuModel();
  }

  // set m_stopped flag (notify StopMainLoop() that the thread has exited)
  m_refresh_thread_stopped = true;
}

} // namespace qtgmenu
