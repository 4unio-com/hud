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

#include <gtest/gtest.h>
#include <QSignalSpy>

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
      : m_importer( c_service, c_path )
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

  int GetMenuItemCount()
  {
    GMenuModel* menu = m_importer.GetGMenuModel();

    if( !menu )
    {
      return 0;
    }

    gint item_count = g_menu_model_get_n_items( G_MENU_MODEL( menu ) );

    return item_count;
  }

  int GetActionCount()
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

  constexpr static const char* c_service = "com.canonical.qtgmenu";
  constexpr static const char* c_path = "/com/canonical/qtgmenu";

  QtGMenuImporter m_importer;
  GDBusConnection* m_connection;
  GMenu* m_menu;
  GSimpleActionGroup* m_actions;
};

TEST_F(TestQtGMenu, ExportImportMenu)
{
  QSignalSpy items_changed_spy( &m_importer, SIGNAL( MenuItemsChanged() ) );
  QSignalSpy menu_appeared_spy( &m_importer, SIGNAL( MenuAppeared() ) );
  QSignalSpy menu_disappeared_spy( &m_importer, SIGNAL( MenuDisappeared() ) );

  // no menu exported

  g_menu_append( m_menu, "New", "app.new" );

  EXPECT_EQ( 0, m_importer.GetQMenus().size() );

  // export menu

  guint export_id = g_dbus_connection_export_menu_model( m_connection, c_path,
      G_MENU_MODEL( m_menu ), NULL );

  menu_appeared_spy.wait();
  EXPECT_FALSE( menu_appeared_spy.empty() );
  menu_appeared_spy.clear();

  EXPECT_NE( 0, m_importer.GetQMenus().size() );
  EXPECT_EQ( 1, GetMenuItemCount() );

  // add 2 items

  g_menu_append( m_menu, "Add", "app.add" );
  items_changed_spy.wait();
  EXPECT_FALSE( items_changed_spy.empty() );
  items_changed_spy.clear();

  g_menu_append( m_menu, "Del", "app.del" );
  items_changed_spy.wait();
  EXPECT_FALSE( items_changed_spy.empty() );
  items_changed_spy.clear();

  EXPECT_EQ( 3, GetMenuItemCount() );

  // remove 1 item

  g_menu_remove( m_menu, 2 );
  items_changed_spy.wait();
  EXPECT_FALSE( items_changed_spy.empty() );
  items_changed_spy.clear();

  EXPECT_EQ( 2, GetMenuItemCount() );

  // unexport menu

  g_dbus_connection_unexport_menu_model( m_connection, export_id );

  m_importer.ForceRefresh();
  menu_disappeared_spy.wait();
  EXPECT_FALSE( menu_disappeared_spy.empty() );
  menu_disappeared_spy.clear();

  EXPECT_EQ( 0, m_importer.GetQMenus().size() );
  EXPECT_EQ( 0, GetMenuItemCount() );
}

TEST_F(TestQtGMenu, ExportImportActions)
{
  QSignalSpy action_added_spy( &m_importer, SIGNAL( ActionAdded( QString ) ) );
  QSignalSpy action_removed_spy( &m_importer, SIGNAL( ActionRemoved( QString ) ) );
  QSignalSpy action_enabled_spy( &m_importer, SIGNAL( ActionEnabled( QString, bool ) ) );
  QSignalSpy action_state_changed_spy( &m_importer,
      SIGNAL( ActionStateChanged( QString, QVariant ) ) );
  QSignalSpy actions_appeared_spy( &m_importer, SIGNAL( ActionsAppeared() ) );
  QSignalSpy actions_disappeared_spy( &m_importer, SIGNAL( ActionsDisappeared() ) );

  // no actions exported

  GSimpleAction* action = g_simple_action_new_stateful( "app.new", nullptr,
      g_variant_new_boolean( false ) );
  g_action_map_add_action( G_ACTION_MAP( m_actions ), G_ACTION( action ) );

  EXPECT_EQ( nullptr, m_importer.GetGActionGroup() );

  // export actions

  guint export_id = g_dbus_connection_export_action_group( m_connection, c_path,
      G_ACTION_GROUP( m_actions ), NULL );

  actions_appeared_spy.wait();
  EXPECT_FALSE( actions_appeared_spy.empty() );
  actions_appeared_spy.clear();

  EXPECT_NE( nullptr, m_importer.GetGActionGroup() );
  EXPECT_EQ( 1, GetActionCount() );

  // disable / enable action

  g_simple_action_set_enabled( action, false );
  action_enabled_spy.wait();
  EXPECT_FALSE( action_enabled_spy.empty() );
  action_enabled_spy.clear();

  g_simple_action_set_enabled( action, true );
  action_enabled_spy.wait();
  EXPECT_FALSE( action_enabled_spy.empty() );
  action_enabled_spy.clear();

  // change action state

  g_action_change_state( G_ACTION( action ), g_variant_new_boolean( true ) );
  action_state_changed_spy.wait();
  EXPECT_FALSE( action_state_changed_spy.empty() );
  action_state_changed_spy.clear();

  // add 2 actions

  action = g_simple_action_new_stateful( "app.add", G_VARIANT_TYPE_BOOLEAN, FALSE );
  g_action_map_add_action( G_ACTION_MAP( m_actions ), G_ACTION( action ) );
  action_added_spy.wait();
  EXPECT_FALSE( action_added_spy.empty() );
  action_added_spy.clear();

  action = g_simple_action_new_stateful( "app.del", G_VARIANT_TYPE_BOOLEAN, FALSE );
  g_action_map_add_action( G_ACTION_MAP( m_actions ), G_ACTION( action ) );
  action_added_spy.wait();
  EXPECT_FALSE( action_added_spy.empty() );
  action_added_spy.clear();

  EXPECT_EQ( 3, GetActionCount() );

  // remove 1 action

  g_action_map_remove_action( G_ACTION_MAP( m_actions ), "app.del" );
  action_removed_spy.wait();
  EXPECT_FALSE( action_removed_spy.empty() );
  action_removed_spy.clear();

  EXPECT_EQ( 2, GetActionCount() );

  // unexport actions

  g_dbus_connection_unexport_action_group( m_connection, export_id );

  m_importer.ForceRefresh();
  actions_disappeared_spy.wait();
  EXPECT_FALSE( actions_disappeared_spy.empty() );
  actions_disappeared_spy.clear();

  EXPECT_EQ( nullptr, m_importer.GetGActionGroup() );
  EXPECT_EQ( 0, GetActionCount() );
}

} // namespace
