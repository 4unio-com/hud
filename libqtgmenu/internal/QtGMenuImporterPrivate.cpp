#include <QtGMenuImporterPrivate.h>
#include <QtGMenuConverter.h>

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
    : QObject( 0 ),
      m_parent( parent ),
      m_connection( g_bus_get_sync( G_BUS_TYPE_SESSION, NULL, NULL ) ),
      m_service( service.toStdString() ),
      m_path( path.toStdString() ),
      m_gmenu_model( nullptr ),
      m_qmenu( new QMenu() )
{
}

QtGMenuImporterPrivate::~QtGMenuImporterPrivate()
{
  StopPollingThread();

  g_object_unref( m_connection );

  if( m_gmenu_model != nullptr )
  {
    g_object_unref( m_gmenu_model );
  }
}

GMenuModel* QtGMenuImporterPrivate::GetGMenuModel()
{
  std::lock_guard< std::mutex > lock( m_gmenu_model_mutex );

  if( m_gmenu_model == nullptr )
  {
    return nullptr;
  }

  GMenu* menu = g_menu_new();
  gint item_count = g_menu_model_get_n_items( m_gmenu_model );

  for( gint i = 0; i < item_count; ++i )
  {
    g_menu_append_item( menu, g_menu_item_new_from_model( m_gmenu_model, i ) );
  }

  return G_MENU_MODEL( menu );
}

std::shared_ptr< QMenu > QtGMenuImporterPrivate::GetQMenu()
{
  std::lock_guard< std::mutex > lock( m_gmenu_model_mutex );

  if( m_gmenu_model == nullptr )
  {
    return nullptr;
  }

  return m_qmenu;
}

void QtGMenuImporterPrivate::MenuItemsChanged( GMenuModel* model, gint position, gint removed,
    gint added, gpointer user_data )
{
  unused( model, position, removed, added );
  QtGMenuImporter* importer = reinterpret_cast< QtGMenuImporter* >( user_data );
  emit importer->MenuItemsChanged();
}

void QtGMenuImporterPrivate::MenuRefresh( GMenuModel* model, gint position, gint removed,
    gint added, gpointer user_data )
{
  unused( model, position, removed, added );
  QtGMenuImporterPrivate* importer = reinterpret_cast< QtGMenuImporterPrivate* >( user_data );
  emit importer->MenuRefreshed();
}

bool QtGMenuImporterPrivate::RefreshGMenuModel()
{  
  GMenuModel* model =
      G_MENU_MODEL( g_dbus_menu_model_get( m_connection, m_service.c_str(), m_path.c_str()) );

  gint item_count = g_menu_model_get_n_items( model );

  if( item_count == 0 )
  {
    QEventLoop menu_refresh_wait;
    QTimer timeout;
    gulong menu_refresh_handler =
        g_signal_connect( model, "items-changed", G_CALLBACK( MenuRefresh ), this );

    // block until "items-changed" fires or timeout reached
    menu_refresh_wait.connect( this, SIGNAL( MenuRefreshed() ), SLOT( quit() ) );
    timeout.singleShot( 10, &menu_refresh_wait, SLOT( quit() ) );
    menu_refresh_wait.exec();

    g_signal_handler_disconnect( model, menu_refresh_handler );

    item_count = g_menu_model_get_n_items( model );
  }

  std::lock_guard< std::mutex > lock( m_gmenu_model_mutex );

  bool menu_was_valid = m_gmenu_model != nullptr;
  bool menu_is_valid = item_count != 0;

  if( menu_was_valid == menu_is_valid )
  {
    g_object_unref( model );

    return menu_is_valid;
  }
  else if( !menu_was_valid && menu_is_valid )
  {
    m_gmenu_model = model;
    m_sig_handler =
        g_signal_connect( m_gmenu_model, "items-changed", G_CALLBACK( MenuItemsChanged ), &m_parent );

    emit m_parent.MenuAppeared();
    return true;
  }
  else if( menu_was_valid && !menu_is_valid )
  {
    g_signal_handler_disconnect( m_gmenu_model, m_sig_handler );
    g_object_unref( m_gmenu_model );
    g_object_unref( model );

    m_sig_handler = 0;
    m_gmenu_model = nullptr;

    emit m_parent.MenuDisappeared();
    return false;
  }
}

void QtGMenuImporterPrivate::StopPollingThread()
{
  // set m_thread_stop flag (notify RefreshThread to exit)
  m_thread_stop = true;

  // wait until RefreshThread exits
  while( !m_thread_stopped )
  {
    std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
  }

  // join with m_refresh_thread
  m_refresh_thread.join();
}

void QtGMenuImporterPrivate::PollingThread()
{
  while( !m_thread_stop )
  {
    RefreshGMenuModel();
    QEventLoop menu_refresh_wait;
    QTimer timeout;
    menu_refresh_wait.connect( this, SIGNAL( MenuRefreshed() ), SLOT( quit() ) );
    timeout.singleShot( 10, &menu_refresh_wait, SLOT( quit() ) );
    menu_refresh_wait.exec();
  }

  // set m_thread_stopped flag (notify StopRefreshThread that the thread has exited)
  m_thread_stopped = true;
}

} // namespace qtgmenu

#include <QtGMenuImporterPrivate.moc>
