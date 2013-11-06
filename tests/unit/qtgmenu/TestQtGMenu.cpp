#include <gio/gio.h>
#include <gtest/gtest.h>

#include <libqtgmenu/QtGMenuImporter.h>
#include <QApplication>

#include <thread>

using namespace qtgmenu;

namespace
{

TEST(TestQtGMenu, ExportImportMenu)
{
  g_bus_own_name( G_BUS_TYPE_SESSION, "com.canonical.qtgmenu", G_BUS_NAME_OWNER_FLAGS_NONE, NULL,
      NULL, NULL, NULL, NULL );

  GDBusConnection* connection = g_bus_get_sync( G_BUS_TYPE_SESSION, NULL, NULL );

  GMenu* menu = g_menu_new();
  g_menu_append( menu, "Add", "app.add" );
  g_menu_append( menu, "Del", "app.del" );
  g_menu_append( menu, "Quit", "app.quit" );

  QtGMenuImporter importer( "com.canonical.qtgmenu", "/com/canonical/qtgmenu" );

  auto qmenu = importer.Menu();
  EXPECT_EQ( nullptr, qmenu );

  g_dbus_connection_export_menu_model( connection, "/com/canonical/qtgmenu", G_MENU_MODEL (menu),
      NULL );

  qmenu = importer.Menu();
  EXPECT_NE( nullptr, qmenu );

  g_object_unref( menu );
  g_object_unref( connection );
}

} // namespace
