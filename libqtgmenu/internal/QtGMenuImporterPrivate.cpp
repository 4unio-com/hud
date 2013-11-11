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
      m_qmenu( new QMenu() ),
      m_gmenu_model( nullptr ),
      m_gmenu_actions( nullptr )
{
  connect( &m_menu_poll_timer, SIGNAL( timeout() ), this, SLOT( RefreshGMenuModel() ) );
  connect( &m_actions_poll_timer, SIGNAL( timeout() ), this, SLOT( RefreshGActionGroup() ) );
  StartPolling( 100 );
}

QtGMenuImporterPrivate::~QtGMenuImporterPrivate()
{
  m_menu_poll_timer.stop();

  g_object_unref( m_connection );

  ClearGMenuModel();
}

GMenu* QtGMenuImporterPrivate::GetGMenu()
{
  std::lock_guard < std::mutex > lock( m_menu_poll_mutex );

  if( m_gmenu_model == nullptr )
  {
    return nullptr;
  }

  // return a copy of m_gmenu_model
  GMenu* menu = g_menu_new();
  gint item_count = g_menu_model_get_n_items( m_gmenu_model );

  for( gint i = 0; i < item_count; ++i )
  {
    g_menu_append_item( menu, g_menu_item_new_from_model( m_gmenu_model, i ) );
  }

  return menu;
}

GActionGroup* QtGMenuImporterPrivate::GetGActionGroup()
{
  return nullptr;
}


std::shared_ptr< QMenu > QtGMenuImporterPrivate::GetQMenu()
{
  GMenu* gmenu = GetGMenu();

  if( gmenu == nullptr )
  {
    return nullptr;
  }

  std::shared_ptr< QMenu > qmenu = QtGMenuConverter::ToQMenu( *gmenu );

  g_object_unref( gmenu );

  // return a copy of m_gmenu_model as a QMenu
  return qmenu;
}

void QtGMenuImporterPrivate::StartPolling( int interval )
{
  m_menu_poll_timer.stop();
  m_menu_poll_timer.setInterval( interval );
  m_menu_poll_timer.start();

  m_actions_poll_timer.stop();
  m_actions_poll_timer.setInterval( interval );
  m_actions_poll_timer.start();
}

void QtGMenuImporterPrivate::MenuItemsChangedCallback( GMenuModel* model, gint position, gint removed,
    gint added, gpointer user_data )
{
  unused( model, position, removed, added );
  QtGMenuImporter* importer = reinterpret_cast< QtGMenuImporter* >( user_data );
  emit importer->MenuItemsChanged();
}

void QtGMenuImporterPrivate::ClearGMenuModel()
{
  if( m_gmenu_model == nullptr )
  {
    return;
  }

  gint item_count = g_menu_model_get_n_items( m_gmenu_model );

  if( item_count > 0 )
  {
    // notify parent that all items are being removed
    MenuItemsChangedCallback( m_gmenu_model, 0, item_count, 0, &m_parent );
  }

  g_signal_handler_disconnect( m_gmenu_model, m_menu_sig_handler );
  m_menu_sig_handler = 0;

  g_object_unref( m_gmenu_model );
  m_gmenu_model = nullptr;
}

bool QtGMenuImporterPrivate::RefreshGMenuModel()
{
  std::lock_guard < std::mutex > lock( m_menu_poll_mutex );

  bool menu_was_valid = m_gmenu_model != nullptr;

  // clear the menu model for the refresh
  ClearGMenuModel();

  m_gmenu_model =
      G_MENU_MODEL( g_dbus_menu_model_get( m_connection, m_service.c_str(), m_path.c_str()) );

  m_menu_sig_handler =
      g_signal_connect( m_gmenu_model, "items-changed", G_CALLBACK( MenuItemsChangedCallback ), &m_parent );

  gint item_count = g_menu_model_get_n_items( m_gmenu_model );

  if( item_count == 0 )
  {
    // block until "items-changed" fires or timeout reached
    QEventLoop menu_refresh_wait;
    QTimer timeout;

    menu_refresh_wait.connect( &m_parent, SIGNAL( MenuItemsChanged() ), SLOT( quit() ) );
    timeout.singleShot( 100, &menu_refresh_wait, SLOT( quit() ) );
    menu_refresh_wait.exec();

    // check item count again
    item_count = g_menu_model_get_n_items( m_gmenu_model );
  }

  bool menu_is_valid = item_count != 0;

  if( menu_is_valid )
  {
    // menu is valid, no need to continue polling
    m_menu_poll_timer.stop();
  }
  else if( !menu_is_valid )
  {
    // clear the menu model
    ClearGMenuModel();
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

bool QtGMenuImporterPrivate::RefreshGActionGroup()
{
  std::lock_guard < std::mutex > lock( m_actions_poll_mutex );

  bool actions_were_valid = m_gmenu_actions != nullptr;

  if( actions_were_valid )
  {
    // temporarily unref the m_gmenu_model for the refresh
    //g_signal_handler_disconnect( m_gmenu_actions, m_actions_sig_handler );
    m_actions_sig_handler = 0;

    g_object_unref( m_gmenu_actions );
    m_gmenu_actions = nullptr;
  }

  GActionGroup* actions =
      G_ACTION_GROUP( g_dbus_action_group_get( m_connection, m_service.c_str(), m_path.c_str()) );

  gchar** action_list = g_action_group_list_actions( actions );

//  if( action_list == nullptr )
//  {
//    // block until "items-changed" fires or timeout reached
//    QEventLoop menu_refresh_wait;
//    QTimer timeout;
//    gulong actions_refresh_handler =
//        g_signal_connect( actions, "action-added", G_CALLBACK( MenuRefreshedCallback ), this );

//    menu_refresh_wait.connect( this, SIGNAL( MenuRefreshed() ), SLOT( quit() ) );
//    timeout.singleShot( 100, &menu_refresh_wait, SLOT( quit() ) );
//    menu_refresh_wait.exec();

//    g_signal_handler_disconnect( actions, actions_refresh_handler );

//    // check item count again
//    action_list = g_menu_model_get_n_items( actions );
//  }

  bool actions_are_valid = action_list != nullptr;

  if( actions_are_valid )
  {
    // actions are valid, so assign m_gmenu_actions accordingly
    m_actions_poll_timer.stop();

    m_gmenu_actions = actions;
//    m_actions_sig_handler =
//        g_signal_connect( m_gmenu_model, "items-changed", G_CALLBACK( ItemsChangedCallback ), &m_parent );
  }
  else if( !actions_are_valid )
  {
    g_object_unref( actions );
  }

  if( !actions_were_valid && actions_are_valid )
  {
    //emit m_parent.MenuAppeared();
  }
  else if( actions_were_valid && !actions_are_valid )
  {
    //emit m_parent.MenuDisappeared();
  }

  return actions_are_valid;
}

} // namespace qtgmenu

#include <QtGMenuImporterPrivate.moc>
