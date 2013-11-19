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

#include <QMenu>
#include <QSignalSpy>

#include <gtest/gtest.h>

#undef signals
#include <gio/gio.h>

using namespace qtgmenu;
using namespace testing;

namespace
{

class TestQtGMenu : public Test
{
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
        m_actions_disappeared_spy( &m_importer, SIGNAL( ActionsDisappeared() ) )
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

  void ExportGMenu()
  {
    // build m_menu

    GMenu* menus_section = g_menu_new();

    //-- build file menu

    GMenu* file_submenu = g_menu_new();
    GMenu* files_section = g_menu_new();

    g_menu_append( files_section, "New", "app.new" );
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
    GMenu* style_section = g_menu_new();

    g_menu_append( style_section, "Plain", "app.text_plain" );
    g_menu_append( style_section, "Bold", "app.text_bold" );

    g_menu_append_section( edit_submenu, "Style", G_MENU_MODEL( style_section ) );
    g_object_unref( style_section );

    g_menu_append_submenu( menus_section, "Edit", G_MENU_MODEL( edit_submenu ) );
    g_object_unref( edit_submenu );

    //-- add menus section to m_menu

    g_menu_append_section( m_menu, "", G_MENU_MODEL( menus_section ) );

    g_object_unref( menus_section );

    // define actions

    //-- stateless

    GSimpleAction* action = g_simple_action_new( "app.new", nullptr );
    g_action_map_add_action( G_ACTION_MAP( m_actions ), G_ACTION( action ) );

    action = g_simple_action_new( "app.open", nullptr );
    g_action_map_add_action( G_ACTION_MAP( m_actions ), G_ACTION( action ) );

    //-- boolean state

    action = g_simple_action_new_stateful( "app.lock", nullptr, g_variant_new_boolean( false ) );
    g_action_map_add_action( G_ACTION_MAP( m_actions ), G_ACTION( action ) );

    //-- string param + state

    action = g_simple_action_new_stateful( "app.text_plain", G_VARIANT_TYPE_STRING,
        g_variant_new_string( "app.text_plain" ) );
    g_action_map_add_action( G_ACTION_MAP( m_actions ), G_ACTION( action ) );

    action = g_simple_action_new_stateful( "app.text_bold", G_VARIANT_TYPE_STRING,
        g_variant_new_string( "app.text_plain" ) );
    g_action_map_add_action( G_ACTION_MAP( m_actions ), G_ACTION( action ) );

    // export menu

    m_menu_export_id = g_dbus_connection_export_menu_model( m_connection, c_path,
        G_MENU_MODEL( m_menu ), NULL );
    m_menu_appeared_spy.wait();
    EXPECT_FALSE( m_menu_appeared_spy.empty() );
    m_menu_appeared_spy.clear();

    // export actions

    m_actions_export_id = g_dbus_connection_export_action_group( m_connection, c_path,
        G_ACTION_GROUP( m_actions ), NULL );
    m_actions_appeared_spy.wait();
    EXPECT_FALSE( m_actions_appeared_spy.empty() );
    m_actions_appeared_spy.clear();
  }

  void UnexportGMenu()
  {
    // unexport menu

    g_dbus_connection_unexport_menu_model( m_connection, m_menu_export_id );

    m_importer.ForceRefresh();

    m_menu_disappeared_spy.wait();
    EXPECT_FALSE( m_menu_disappeared_spy.empty() );
    m_menu_disappeared_spy.clear();

    EXPECT_EQ( nullptr, m_importer.GetQMenu() );
    EXPECT_EQ( 0, GetGMenuSize() );

    // unexport actions

    g_dbus_connection_unexport_action_group( m_connection, m_actions_export_id );

    m_importer.ForceRefresh();

    m_actions_disappeared_spy.wait();
    EXPECT_FALSE( m_actions_disappeared_spy.empty() );
    m_actions_disappeared_spy.clear();

    EXPECT_EQ( nullptr, m_importer.GetGActionGroup() );
    EXPECT_EQ( 0, GetGActionCount() );
  }

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
};

TEST_F( TestQtGMenu, ExportImportGMenu )
{
  // no menu exported

  g_menu_append( m_menu, "New", "app.new" );

  EXPECT_EQ( nullptr, m_importer.GetQMenu() );

  // export menu

  m_menu_export_id = g_dbus_connection_export_menu_model( m_connection, c_path,
      G_MENU_MODEL( m_menu ), NULL );

  m_menu_appeared_spy.wait();
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

  m_importer.ForceRefresh();

  m_menu_disappeared_spy.wait();
  EXPECT_FALSE( m_menu_disappeared_spy.empty() );
  m_menu_disappeared_spy.clear();

  EXPECT_EQ( nullptr, m_importer.GetQMenu() );
  EXPECT_EQ( 0, GetGMenuSize() );
}

TEST_F( TestQtGMenu, ExportImportGActions )
{
  // no actions exported

  GSimpleAction* action = g_simple_action_new_stateful( "app.new", nullptr,
      g_variant_new_boolean( false ) );
  g_action_map_add_action( G_ACTION_MAP( m_actions ), G_ACTION( action ) );

  EXPECT_EQ( nullptr, m_importer.GetGActionGroup() );

  // export actions

  m_actions_export_id = g_dbus_connection_export_action_group( m_connection, c_path,
      G_ACTION_GROUP( m_actions ), NULL );

  m_actions_appeared_spy.wait();
  EXPECT_FALSE( m_actions_appeared_spy.empty() );
  m_actions_appeared_spy.clear();

  EXPECT_NE( nullptr, m_importer.GetGActionGroup() );
  EXPECT_EQ( 1, GetGActionCount() );

  // disable / enable action

  g_simple_action_set_enabled( action, false );
  m_action_enabled_spy.wait();
  EXPECT_FALSE( m_action_enabled_spy.empty() );
  m_action_enabled_spy.clear();

  g_simple_action_set_enabled( action, true );
  m_action_enabled_spy.wait();
  EXPECT_FALSE( m_action_enabled_spy.empty() );
  m_action_enabled_spy.clear();

  // change action state

  g_action_change_state( G_ACTION( action ), g_variant_new_boolean( true ) );
  m_action_state_changed_spy.wait();
  EXPECT_FALSE( m_action_state_changed_spy.empty() );
  m_action_state_changed_spy.clear();

  // add 2 actions

  action = g_simple_action_new_stateful( "app.add", G_VARIANT_TYPE_BOOLEAN, FALSE );
  g_action_map_add_action( G_ACTION_MAP( m_actions ), G_ACTION( action ) );
  m_action_added_spy.wait();
  EXPECT_FALSE( m_action_added_spy.empty() );
  m_action_added_spy.clear();

  action = g_simple_action_new_stateful( "app.del", G_VARIANT_TYPE_BOOLEAN, FALSE );
  g_action_map_add_action( G_ACTION_MAP( m_actions ), G_ACTION( action ) );
  m_action_added_spy.wait();
  EXPECT_FALSE( m_action_added_spy.empty() );
  m_action_added_spy.clear();

  EXPECT_EQ( 3, GetGActionCount() );

  // remove 1 action

  g_action_map_remove_action( G_ACTION_MAP( m_actions ), "app.del" );
  m_action_removed_spy.wait();
  EXPECT_FALSE( m_action_removed_spy.empty() );
  m_action_removed_spy.clear();

  EXPECT_EQ( 2, GetGActionCount() );

  // unexport actions

  g_dbus_connection_unexport_action_group( m_connection, m_actions_export_id );

  m_importer.ForceRefresh();

  m_actions_disappeared_spy.wait();
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
  EXPECT_NE( nullptr, m_importer.GetQMenu() );

  EXPECT_EQ( 2, menu->actions().size() );

  UnexportGMenu();
}

TEST_F( TestQtGMenu, QMenuActions )
{
  ExportGMenu();

  // import QMenu

  std::shared_ptr< QMenu > menu = m_importer.GetQMenu();
  EXPECT_NE( nullptr, m_importer.GetQMenu() );

  UnexportGMenu();
}

} // namespace
