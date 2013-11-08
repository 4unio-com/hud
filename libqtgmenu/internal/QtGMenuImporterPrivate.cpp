#include <QtGMenuImporterPrivate.h>
#include <QtGMenuConverter.h>

#include <QEventLoop>

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
  connect( &menu_poll_timer, SIGNAL( timeout() ), this, SLOT( RefreshGMenuModel() ) );
  StartPollTimer( 100 );
}

QtGMenuImporterPrivate::~QtGMenuImporterPrivate()
{
  menu_poll_timer.stop();

  g_object_unref( m_connection );

  if( m_gmenu_model != nullptr )
  {
    g_object_unref( m_gmenu_model );
  }
}

GMenuModel* QtGMenuImporterPrivate::GetGMenuModel()
{
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
  if( m_gmenu_model == nullptr )
  {
    return nullptr;
  }

  return m_qmenu;
}

void QtGMenuImporterPrivate::StartPollTimer( int interval )
{
  menu_poll_timer.setInterval( interval );
  menu_poll_timer.start();
}

void QtGMenuImporterPrivate::ItemsChangedCallback( GMenuModel* model, gint position, gint removed,
    gint added, gpointer user_data )
{
  unused( model, position, removed, added );
  QtGMenuImporter* importer = reinterpret_cast< QtGMenuImporter* >( user_data );
  emit importer->MenuItemsChanged();
}

void QtGMenuImporterPrivate::MenuRefreshedCallback( GMenuModel* model, gint position, gint removed,
    gint added, gpointer user_data )
{
  unused( model, position, removed, added );
  QtGMenuImporterPrivate* importer = reinterpret_cast< QtGMenuImporterPrivate* >( user_data );
  emit importer->MenuRefreshed();
}

bool QtGMenuImporterPrivate::RefreshGMenuModel()
{
  bool menu_was_valid = m_gmenu_model != nullptr;

  if( menu_was_valid )
  {
    // temporarily unref the m_gmenu_model for the refresh
    g_signal_handler_disconnect( m_gmenu_model, m_sig_handler );
    m_sig_handler = 0;

    g_object_unref( m_gmenu_model );
    m_gmenu_model = nullptr;
  }

  GMenuModel* model =
      G_MENU_MODEL( g_dbus_menu_model_get( m_connection, m_service.c_str(), m_path.c_str()) );

  gint item_count = g_menu_model_get_n_items( model );

  if( item_count == 0 )
  {
    // block until "items-changed" fires or timeout reached
    QEventLoop menu_refresh_wait;
    QTimer timeout;
    gulong menu_refresh_handler =
        g_signal_connect( model, "items-changed", G_CALLBACK( MenuRefreshedCallback ), this );

    menu_refresh_wait.connect( this, SIGNAL( MenuRefreshed() ), SLOT( quit() ) );
    timeout.singleShot( 100, &menu_refresh_wait, SLOT( quit() ) );
    menu_refresh_wait.exec();

    g_signal_handler_disconnect( model, menu_refresh_handler );

    // check item count again
    item_count = g_menu_model_get_n_items( model );
  }

  bool menu_is_valid = item_count != 0;

  if( menu_is_valid )
  {
    // menu is valid, so assign m_gmenu_model accordingly
    menu_poll_timer.stop();

    m_gmenu_model = model;
    m_sig_handler =
        g_signal_connect( m_gmenu_model, "items-changed", G_CALLBACK( ItemsChangedCallback ), &m_parent );
  }
  else if( !menu_is_valid )
  {
    g_object_unref( model );
  }

  if( !menu_was_valid && menu_is_valid )
  {
    emit m_parent.MenuAppeared();
  }
  else if( menu_was_valid && !menu_is_valid )
  {
    emit m_parent.MenuDisappeared();
  }

  return menu_is_valid;
}

} // namespace qtgmenu

#include <QtGMenuImporterPrivate.moc>
