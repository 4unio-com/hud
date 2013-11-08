#include <gio/gio.h>
#include <gtest/gtest.h>

#include <libqtgmenu/QtGMenuImporter.h>
#include <QApplication>

#include <QSignalSpy>

using namespace qtgmenu;

namespace
{

TEST(TestQtGMenu, ExportImportMenu)
{
  QtGMenuImporter importer( "com.canonical.qtgmenu", "/com/canonical/qtgmenu" );

  QSignalSpy items_changed_spy( &importer, SIGNAL( MenuItemsChanged() ) );
  QSignalSpy menu_appeared_spy( &importer, SIGNAL( MenuAppeared() ) );
  QSignalSpy menu_disappeared_spy( &importer, SIGNAL( MenuDisappeared() ) );

  g_bus_own_name( G_BUS_TYPE_SESSION, "com.canonical.qtgmenu", G_BUS_NAME_OWNER_FLAGS_NONE, NULL,
      NULL, NULL, NULL, NULL );

  GDBusConnection* connection = g_bus_get_sync( G_BUS_TYPE_SESSION, NULL, NULL );

  GMenu* menu = g_menu_new();
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

  g_object_unref( menu );
  g_object_unref( connection );
}

} // namespace
