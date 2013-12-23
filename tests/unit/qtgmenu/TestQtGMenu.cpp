/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Marcus Tomlinson <marcus.tomlinson@canonical.com>
 */

#include <libqtgmenu/QtGMenuImporter.h>
#include <libqtgmenu/internal/QtGMenuUtils.h>

#include <QMenu>
#include <QSignalSpy>
#include <QTimer>

#include <gtest/gtest.h>

#undef signals
#include <gio/gio.h>

using namespace qtgmenu;
using namespace testing;

namespace
{

class TestQtGMenu : public QObject,
                    public Test
{
Q_OBJECT

protected:
  TestQtGMenu()
      : m_importer( c_service, c_path ),

        m_items_changed_spy( &m_importer, SIGNAL( MenuItemsChanged() ) ),
        m_menu_appeared_spy( &m_importer, SIGNAL( MenuAppeared() ) ),
        m_menu_disappeared_spy( &m_importer, SIGNAL( MenuDisappeared() ) ),

        m_action_added_spy( &m_importer, SIGNAL( ActionAdded( QString ) ) ),
        m_action_removed_spy( &m_importer, SIGNAL( ActionRemoved( QString ) ) ),
        m_action_enabled_spy( &m_importer, SIGNAL( ActionEnabled( QString, bool ) ) ),
        m_action_state_changed_spy( &m_importer,
            SIGNAL( ActionStateChanged( QString, QVariant ) ) ),
        m_actions_appeared_spy( &m_importer, SIGNAL( ActionsAppeared() ) ),
        m_actions_disappeared_spy( &m_importer, SIGNAL( ActionsDisappeared() ) ),

        m_action_activated_spy( this, SIGNAL( ActionActivated( QString, QVariant ) ) )
  {
    g_bus_own_name( G_BUS_TYPE_SESSION, c_service, G_BUS_NAME_OWNER_FLAGS_NONE, NULL, NULL, NULL,
        NULL, NULL );

    m_connection = g_bus_get_sync( G_BUS_TYPE_SESSION, NULL, NULL );
    m_menu = g_menu_new();
    m_actions = g_simple_action_group_new();
  }

  virtual ~TestQtGMenu()
  {
    g_object_unref( m_actions );
    g_object_unref( m_menu );
    g_object_unref( m_connection );
  }

  int GetGMenuSize()
  {
    GMenuModel* menu = m_importer.GetGMenuModel();

    if( !menu )
    {
      return 0;
    }

    gint item_count = g_menu_model_get_n_items( G_MENU_MODEL( menu ) );

    return item_count;
  }

  int GetGActionCount()
  {
    GActionGroup* actions = m_importer.GetGActionGroup();

    if( !actions )
    {
      return 0;
    }

    gchar** actions_list = g_action_group_list_actions( actions );

    int action_count = 0;
    while( actions_list[action_count] != nullptr )
    {
      ++action_count;
    }

    g_strfreev( actions_list );
    return action_count;
  }

  void RefreshImporter()
  {
    // process any outstanding events (e.g. export / unexport)
    QCoreApplication::processEvents();

    // refresh the importer
    m_importer.Refresh();

    // wait for "items-changed" and "action-added" signals to settle
    QEventLoop refresh_wait;
    QTimer timeout;
    timeout.singleShot( 100, &refresh_wait, SLOT( quit() ) );
    refresh_wait.exec();
  }

  static void ActivateAction( GSimpleAction* simple, GVariant* parameter, gpointer user_data )
  {
    TestQtGMenu* self = reinterpret_cast< TestQtGMenu* >( user_data );

    const gchar* action_name = g_action_get_name( G_ACTION( simple ) );
    emit self->ActionActivated( action_name, QtGMenuUtils::GVariantToQVariant( parameter ) );
  }

  void ExportGMenu()
  {
    // build m_menu

    GMenu* menus_section = g_menu_new();

    //-- build file menu

    GMenu* file_submenu = g_menu_new();
    GMenu* files_section = g_menu_new();

    GMenuItem* menu_item = g_menu_item_new( "New", "app.new" );
    g_menu_item_set_attribute_value( menu_item, "accel", g_variant_new_string( "<Control>N" ) );
    g_menu_append_item( files_section, menu_item );
    g_object_unref( menu_item );

    g_menu_append( files_section, "Open", "app.open" );

    g_menu_append_section( file_submenu, "Files", G_MENU_MODEL( files_section ) );
    g_object_unref( files_section );

    GMenu* mod_section = g_menu_new();

    g_menu_append( mod_section, "Lock", "app.lock" );

    g_menu_append_section( file_submenu, "Modify", G_MENU_MODEL( mod_section ) );
    g_object_unref( mod_section );

    g_menu_append_submenu( menus_section, "File", G_MENU_MODEL( file_submenu ) );
    g_object_unref( file_submenu );

    //-- build edit menu

    GMenu* edit_submenu = g_menu_new();
    GMenu* style_submenu = g_menu_new();

    g_menu_append( style_submenu, "Plain", "app.text_plain" );
    g_menu_append( style_submenu, "Bold", "app.text_bold" );

    g_menu_append_submenu( edit_submenu, "Style", G_MENU_MODEL( style_submenu ) );
    g_object_unref( style_submenu );

    g_menu_append_submenu( menus_section, "Edit", G_MENU_MODEL( edit_submenu ) );
    g_object_unref( edit_submenu );

    //-- add menus section to m_menu

    g_menu_append_section( m_menu, "Menus", G_MENU_MODEL( menus_section ) );

    g_object_unref( menus_section );

    // export menu

    m_menu_export_id = g_dbus_connection_export_menu_model( m_connection, c_path,
        G_MENU_MODEL( m_menu ), NULL );

    RefreshImporter();

    EXPECT_FALSE( m_menu_appeared_spy.empty() );
    m_menu_appeared_spy.clear();

    // define actions

    //-- stateless

    GSimpleAction* action = g_simple_action_new( "new", nullptr );
    m_exported_actions.push_back(
        std::make_pair( action,
            g_signal_connect( action, "activate", G_CALLBACK( ActivateAction ), this ) ) );
    g_action_map_add_action( G_ACTION_MAP( m_actions ), G_ACTION( action ) );

    action = g_simple_action_new( "open", nullptr );
    m_exported_actions.push_back(
        std::make_pair( action,
            g_signal_connect( action, "activate", G_CALLBACK( ActivateAction ), this ) ) );
    g_action_map_add_action( G_ACTION_MAP( m_actions ), G_ACTION( action ) );

    //-- boolean state

    action = g_simple_action_new_stateful( "lock", nullptr, g_variant_new_boolean( false ) );
    m_exported_actions.push_back(
        std::make_pair( action,
            g_signal_connect( action, "activate", G_CALLBACK( ActivateAction ), this ) ) );
    g_action_map_add_action( G_ACTION_MAP( m_actions ), G_ACTION( action ) );

    //-- string param + state

    action = g_simple_action_new_stateful( "text_plain", G_VARIANT_TYPE_STRING,
        g_variant_new_string( "app.text_plain" ) );
    m_exported_actions.push_back(
        std::make_pair( action,
            g_signal_connect( action, "activate", G_CALLBACK( ActivateAction ), this ) ) );
    g_action_map_add_action( G_ACTION_MAP( m_actions ), G_ACTION( action ) );

    action = g_simple_action_new_stateful( "text_bold", G_VARIANT_TYPE_STRING,
        g_variant_new_string( "app.text_plain" ) );
    m_exported_actions.push_back(
        std::make_pair( action,
            g_signal_connect( action, "activate", G_CALLBACK( ActivateAction ), this ) ) );
    g_action_map_add_action( G_ACTION_MAP( m_actions ), G_ACTION( action ) );

    // export actions

    m_actions_export_id = g_dbus_connection_export_action_group( m_connection, c_path,
        G_ACTION_GROUP( m_actions ), NULL );

    RefreshImporter();

    EXPECT_FALSE( m_actions_appeared_spy.empty() );
    m_actions_appeared_spy.clear();
  }

  void UnexportGMenu()
  {
    // disconnect signal handlers
    for( auto& action : m_exported_actions )
    {
      g_signal_handler_disconnect( action.first, action.second );
    }

    // unexport menu

    g_dbus_connection_unexport_menu_model( m_connection, m_menu_export_id );

    RefreshImporter();

    EXPECT_FALSE( m_menu_disappeared_spy.empty() );
    m_menu_disappeared_spy.clear();

    EXPECT_EQ( nullptr, m_importer.GetQMenu() );
    EXPECT_EQ( 0, GetGMenuSize() );

    // unexport actions

    g_dbus_connection_unexport_action_group( m_connection, m_actions_export_id );

    RefreshImporter();

    EXPECT_FALSE( m_actions_disappeared_spy.empty() );
    m_actions_disappeared_spy.clear();

    EXPECT_EQ( nullptr, m_importer.GetGActionGroup() );
    EXPECT_EQ( 0, GetGActionCount() );
  }

Q_SIGNALS:
  void ActionActivated( QString action_name, QVariant parameter );

protected:
  constexpr static const char* c_service = "com.canonical.qtgmenu";
  constexpr static const char* c_path = "/com/canonical/qtgmenu";

  guint m_menu_export_id = 0;
  guint m_actions_export_id = 0;

  GDBusConnection* m_connection = nullptr;
  GMenu* m_menu = nullptr;
  GSimpleActionGroup* m_actions = nullptr;

  QtGMenuImporter m_importer;

  QSignalSpy m_items_changed_spy;
  QSignalSpy m_menu_appeared_spy;
  QSignalSpy m_menu_disappeared_spy;

  QSignalSpy m_action_added_spy;
  QSignalSpy m_action_removed_spy;
  QSignalSpy m_action_enabled_spy;
  QSignalSpy m_action_state_changed_spy;
  QSignalSpy m_actions_appeared_spy;
  QSignalSpy m_actions_disappeared_spy;

  QSignalSpy m_action_activated_spy;

  std::vector< std::pair< GSimpleAction*, gulong > > m_exported_actions;
};

TEST_F( TestQtGMenu, ExportImportGMenu )
{
  // no menu exported

  g_menu_append( m_menu, "New", "app.new" );

  EXPECT_EQ( nullptr, m_importer.GetQMenu() );

  // export menu

  m_menu_export_id = g_dbus_connection_export_menu_model( m_connection, c_path,
      G_MENU_MODEL( m_menu ), NULL );

  RefreshImporter();

  EXPECT_FALSE( m_menu_appeared_spy.empty() );
  m_menu_appeared_spy.clear();

  EXPECT_NE( nullptr, m_importer.GetQMenu() );
  EXPECT_EQ( 1, GetGMenuSize() );

  // add 2 items

  g_menu_append( m_menu, "Add", "app.add" );
  m_items_changed_spy.wait();
  EXPECT_FALSE( m_items_changed_spy.empty() );
  m_items_changed_spy.clear();

  g_menu_append( m_menu, "Del", "app.del" );
  m_items_changed_spy.wait();
  EXPECT_FALSE( m_items_changed_spy.empty() );
  m_items_changed_spy.clear();

  EXPECT_EQ( 3, GetGMenuSize() );

  // remove 1 item

  g_menu_remove( m_menu, 2 );
  m_items_changed_spy.wait();
  EXPECT_FALSE( m_items_changed_spy.empty() );
  m_items_changed_spy.clear();

  EXPECT_EQ( 2, GetGMenuSize() );

  // unexport menu

  g_dbus_connection_unexport_menu_model( m_connection, m_menu_export_id );

  RefreshImporter();

  EXPECT_FALSE( m_menu_disappeared_spy.empty() );
  m_menu_disappeared_spy.clear();

  EXPECT_EQ( nullptr, m_importer.GetQMenu() );
  EXPECT_EQ( 0, GetGMenuSize() );
}

TEST_F( TestQtGMenu, ExportImportGActions )
{
  // no actions exported

  GSimpleAction* action = g_simple_action_new_stateful( "new", nullptr,
      g_variant_new_boolean( false ) );
  g_action_map_add_action( G_ACTION_MAP( m_actions ), G_ACTION( action ) );

  EXPECT_EQ( nullptr, m_importer.GetGActionGroup() );

  // export actions

  m_actions_export_id = g_dbus_connection_export_action_group( m_connection, c_path,
      G_ACTION_GROUP( m_actions ), NULL );

  RefreshImporter();

  EXPECT_FALSE( m_actions_appeared_spy.empty() );
  m_actions_appeared_spy.clear();

  EXPECT_FALSE( m_action_added_spy.empty() );
  EXPECT_EQ( "new", m_action_added_spy.at( 0 ).at( 0 ).toString().toStdString() );
  m_action_added_spy.clear();

  EXPECT_NE( nullptr, m_importer.GetGActionGroup() );
  EXPECT_EQ( 1, GetGActionCount() );

  // disable / enable action

  g_simple_action_set_enabled( action, false );
  m_action_enabled_spy.wait();

  EXPECT_FALSE( m_action_enabled_spy.empty() );
  EXPECT_EQ( "new", m_action_enabled_spy.at( 0 ).at( 0 ).toString().toStdString() );
  EXPECT_EQ( "false", m_action_enabled_spy.at( 0 ).at( 1 ).toString().toStdString() );
  m_action_enabled_spy.clear();

  g_simple_action_set_enabled( action, true );
  m_action_enabled_spy.wait();

  EXPECT_FALSE( m_action_enabled_spy.empty() );
  EXPECT_EQ( "new", m_action_enabled_spy.at( 0 ).at( 0 ).toString().toStdString() );
  EXPECT_EQ( "true", m_action_enabled_spy.at( 0 ).at( 1 ).toString().toStdString() );
  m_action_enabled_spy.clear();

  // change action state

  g_action_change_state( G_ACTION( action ), g_variant_new_boolean( true ) );
  m_action_state_changed_spy.wait();

  EXPECT_FALSE( m_action_state_changed_spy.empty() );
  EXPECT_EQ( "new", m_action_state_changed_spy.at( 0 ).at( 0 ).toString().toStdString() );
  EXPECT_EQ( "true", m_action_state_changed_spy.at( 0 ).at( 1 ).toString().toStdString() );
  m_action_state_changed_spy.clear();

  g_action_change_state( G_ACTION( action ), g_variant_new_boolean( false ) );
  m_action_state_changed_spy.wait();

  EXPECT_FALSE( m_action_state_changed_spy.empty() );
  EXPECT_EQ( "new", m_action_state_changed_spy.at( 0 ).at( 0 ).toString().toStdString() );
  EXPECT_EQ( "false", m_action_state_changed_spy.at( 0 ).at( 1 ).toString().toStdString() );
  m_action_state_changed_spy.clear();

  // add 2 actions

  action = g_simple_action_new_stateful( "add", G_VARIANT_TYPE_BOOLEAN, FALSE );
  g_action_map_add_action( G_ACTION_MAP( m_actions ), G_ACTION( action ) );
  m_action_added_spy.wait();

  EXPECT_FALSE( m_action_added_spy.empty() );
  EXPECT_EQ( "add", m_action_added_spy.at( 0 ).at( 0 ).toString().toStdString() );
  m_action_added_spy.clear();

  action = g_simple_action_new_stateful( "del", G_VARIANT_TYPE_BOOLEAN, FALSE );
  g_action_map_add_action( G_ACTION_MAP( m_actions ), G_ACTION( action ) );
  m_action_added_spy.wait();

  EXPECT_FALSE( m_action_added_spy.empty() );
  EXPECT_EQ( "del", m_action_added_spy.at( 0 ).at( 0 ).toString().toStdString() );
  m_action_added_spy.clear();

  EXPECT_EQ( 3, GetGActionCount() );

  // remove 1 action

  g_action_map_remove_action( G_ACTION_MAP( m_actions ), "del" );
  m_action_removed_spy.wait();

  EXPECT_FALSE( m_action_removed_spy.empty() );
  EXPECT_EQ( "del", m_action_removed_spy.at( 0 ).at( 0 ).toString().toStdString() );
  m_action_removed_spy.clear();

  EXPECT_EQ( 2, GetGActionCount() );

  // unexport actions

  g_dbus_connection_unexport_action_group( m_connection, m_actions_export_id );

  RefreshImporter();

  EXPECT_FALSE( m_actions_disappeared_spy.empty() );
  m_actions_disappeared_spy.clear();

  EXPECT_EQ( nullptr, m_importer.GetGActionGroup() );
  EXPECT_EQ( 0, GetGActionCount() );
}

TEST_F( TestQtGMenu, QMenuStructure )
{
  ExportGMenu();

  // import QMenu

  std::shared_ptr< QMenu > menu = m_importer.GetQMenu();
  ASSERT_NE( nullptr, menu );

  EXPECT_EQ( 2, menu->actions().size() );

  EXPECT_EQ( "File", menu->actions().at( 0 )->text() );
  EXPECT_EQ( "Edit", menu->actions().at( 1 )->text() );

  // check file menu structure

  QMenu* file_menu = menu->actions().at( 0 )->menu();
  ASSERT_NE( nullptr, file_menu );

  EXPECT_EQ( 4, file_menu->actions().size() );

  EXPECT_EQ( "File", file_menu->title() );

  EXPECT_EQ( "New", file_menu->actions().at( 0 )->text() );
  EXPECT_EQ( "Ctrl+N", file_menu->actions().at( 0 )->shortcut().toString().toStdString() );

  EXPECT_EQ( "Open", file_menu->actions().at( 1 )->text() );
  EXPECT_TRUE( file_menu->actions().at( 2 )->isSeparator() );
  EXPECT_EQ( "Lock", file_menu->actions().at( 3 )->text() );

  // check edit menu structure

  QMenu* edit_menu = menu->actions().at( 1 )->menu();
  ASSERT_NE( nullptr, edit_menu );

  EXPECT_EQ( 1, edit_menu->actions().size() );

  EXPECT_EQ( "Edit", edit_menu->title() );

  EXPECT_EQ( "Style", edit_menu->actions().at( 0 )->text() );

  // check style submenu structure

  QMenu* style_submenu = edit_menu->actions().at( 0 )->menu();
  ASSERT_NE( nullptr, style_submenu );

  EXPECT_EQ( 2, style_submenu->actions().size() );

  EXPECT_EQ( "Style", style_submenu->title() );

  EXPECT_EQ( "Plain", style_submenu->actions().at( 0 )->text() );
  EXPECT_EQ( "Bold", style_submenu->actions().at( 1 )->text() );

  UnexportGMenu();
}

TEST_F( TestQtGMenu, QMenuActionTriggers )
{
  ExportGMenu();

  // import QMenu

  std::shared_ptr< QMenu > menu = m_importer.GetQMenu();
  EXPECT_NE( nullptr, m_importer.GetQMenu() );

  // trigger file menu items

  QMenu* file_menu = menu->actions().at( 0 )->menu();
  ASSERT_NE( nullptr, file_menu );

  file_menu->actions().at( 0 )->trigger();
  m_action_activated_spy.wait();

  EXPECT_FALSE( m_action_activated_spy.empty() );
  EXPECT_EQ( "new", m_action_activated_spy.at( 0 ).at( 0 ).toString().toStdString() );
  EXPECT_EQ( "", m_action_activated_spy.at( 0 ).at( 1 ).toString().toStdString() );
  m_action_activated_spy.clear();

  file_menu->actions().at( 1 )->trigger();
  m_action_activated_spy.wait();

  EXPECT_FALSE( m_action_activated_spy.empty() );
  EXPECT_EQ( "open", m_action_activated_spy.at( 0 ).at( 0 ).toString().toStdString() );
  EXPECT_EQ( "", m_action_activated_spy.at( 0 ).at( 1 ).toString().toStdString() );
  m_action_activated_spy.clear();

  file_menu->actions().at( 3 )->trigger();
  m_action_activated_spy.wait();

  EXPECT_FALSE( m_action_activated_spy.empty() );
  EXPECT_EQ( "lock", m_action_activated_spy.at( 0 ).at( 0 ).toString().toStdString() );
  EXPECT_EQ( "", m_action_activated_spy.at( 0 ).at( 1 ).toString().toStdString() );
  m_action_activated_spy.clear();

  // trigger edit menu items

  QMenu* edit_menu = menu->actions().at( 1 )->menu();
  ASSERT_NE( nullptr, edit_menu );
  QMenu* style_submenu = edit_menu->actions().at( 0 )->menu();
  ASSERT_NE( nullptr, style_submenu );

  style_submenu->actions().at( 0 )->trigger();
  m_action_activated_spy.wait();

  EXPECT_FALSE( m_action_activated_spy.empty() );
  EXPECT_EQ( "text_plain", m_action_activated_spy.at( 0 ).at( 0 ).toString().toStdString() );
  EXPECT_EQ( "text_plain", m_action_activated_spy.at( 0 ).at( 1 ).toString().toStdString() );
  m_action_activated_spy.clear();

  style_submenu->actions().at( 1 )->trigger();
  m_action_activated_spy.wait();

  EXPECT_FALSE( m_action_activated_spy.empty() );
  EXPECT_EQ( "text_bold", m_action_activated_spy.at( 0 ).at( 0 ).toString().toStdString() );
  EXPECT_EQ( "text_bold", m_action_activated_spy.at( 0 ).at( 1 ).toString().toStdString() );
  m_action_activated_spy.clear();

  UnexportGMenu();
}

TEST_F( TestQtGMenu, QMenuActionStates )
{
  ExportGMenu();

  // import QMenu

  std::shared_ptr< QMenu > menu = m_importer.GetQMenu();
  EXPECT_NE( nullptr, m_importer.GetQMenu() );

  // enable / disable menu items

  QMenu* file_menu = menu->actions().at( 0 )->menu();
  ASSERT_NE( nullptr, file_menu );

  EXPECT_TRUE( file_menu->actions().at( 0 )->isEnabled() );

  g_simple_action_set_enabled( m_exported_actions[0].first, false );
  m_action_enabled_spy.wait();

  EXPECT_FALSE( file_menu->actions().at( 0 )->isEnabled() );

  g_simple_action_set_enabled( m_exported_actions[0].first, true );
  m_action_enabled_spy.wait();

  EXPECT_TRUE( file_menu->actions().at( 0 )->isEnabled() );

  UnexportGMenu();
}

} // namespace

#include <TestQtGMenu.moc>
