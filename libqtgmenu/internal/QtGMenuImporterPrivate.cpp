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
  if( !RefreshGMenuModel() )
  {
    return nullptr;
  }

  if( !RefreshQMenu() )
  {
    return nullptr;
  }

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
  if( m_gmenu_model != nullptr )
  {
    return true;
  }

  m_gmenu_model =
      G_MENU_MODEL( g_dbus_menu_model_get( m_connection, m_service.c_str(), m_path.c_str()) );

  gulong sig_handler =
      g_signal_connect( m_gmenu_model, "items-changed", G_CALLBACK( MenuItemsChanged ), &m_parent );

  gint item_count = g_menu_model_get_n_items( m_gmenu_model );

  if( item_count == 0 )
  {
    QEventLoop loop;
    QTimer timeout;
    loop.connect( &m_parent, SIGNAL( MenuItemsChanged() ), SLOT( quit() ) );
    timeout.singleShot( 10, &loop, SLOT( quit() ) );
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

bool QtGMenuImporterPrivate::RefreshQMenu()
{
  if( m_gmenu_model == nullptr )
  {
    return false;
  }

  // convert m_gmenu_model to m_qmenu
  return true;
}


} // namespace qtgmenu
