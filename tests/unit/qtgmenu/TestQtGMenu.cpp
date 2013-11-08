#include <libqtgmenu/QtGMenuImporter.h>

#include <gtest/gtest.h>
#include <QSignalSpy>

#undef signals
#include <gio/gio.h>

using namespace qtgmenu;
using namespace testing;

namespace
{

class TestQtGMenu: public Test {
protected:
  TestQtGMenu()
    : importer( "com.canonical.qtgmenu", "/com/canonical/qtgmenu" )
  {
    g_bus_own_name( G_BUS_TYPE_SESSION, "com.canonical.qtgmenu", G_BUS_NAME_OWNER_FLAGS_NONE, NULL,
        NULL, NULL, NULL, NULL );

    connection = g_bus_get_sync( G_BUS_TYPE_SESSION, NULL, NULL );
    menu = g_menu_new();
  }

  virtual ~TestQtGMenu()
  {
    g_object_unref( menu );
    g_object_unref( connection );
  }

  QtGMenuImporter importer;
  GDBusConnection* connection;
  GMenu* menu;
};

TEST_F(TestQtGMenu, ExportImportMenu)
{
  QSignalSpy items_changed_spy( &importer, SIGNAL( MenuItemsChanged() ) );
  QSignalSpy menu_appeared_spy( &importer, SIGNAL( MenuAppeared() ) );
  QSignalSpy menu_disappeared_spy( &importer, SIGNAL( MenuDisappeared() ) );

  // no menu exported

  g_menu_append( menu, "New", "app.new" );

  auto qmenu = importer.Menu();
  EXPECT_EQ( nullptr, qmenu );

  // export menu

  guint export_id = g_dbus_connection_export_menu_model( connection, "/com/canonical/qtgmenu",
      G_MENU_MODEL (menu), NULL );

  menu_appeared_spy.wait();

  qmenu = importer.Menu();
  EXPECT_NE( nullptr, qmenu );

  int item_count = importer.GetItemCount();
  EXPECT_EQ( 1, item_count );

  // add 2 items

  g_menu_append( menu, "Add", "app.add" );
  items_changed_spy.wait();

  g_menu_append( menu, "Del", "app.del" );
  items_changed_spy.wait();

  item_count = importer.GetItemCount();
  EXPECT_EQ( 3, item_count );

  // add 1 items

  g_menu_append( menu, "Quit", "app.quit" );
  items_changed_spy.wait();

  item_count = importer.GetItemCount();
  EXPECT_EQ( 4, item_count );

  // unexport menu

  g_dbus_connection_unexport_menu_model( connection, export_id );

  importer.ForceRefresh();
  menu_disappeared_spy.wait();

  qmenu = importer.Menu();
  EXPECT_EQ( nullptr, qmenu );

  item_count = importer.GetItemCount();
  EXPECT_EQ( 0, item_count );
}

} // namespace
