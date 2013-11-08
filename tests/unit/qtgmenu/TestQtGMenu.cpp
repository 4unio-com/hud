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
  }

  virtual ~TestQtGMenu()
  {
    g_object_unref( m_menu );
    g_object_unref( m_connection );
  }

  constexpr static const char* c_service = "com.canonical.qtgmenu";
  constexpr static const char* c_path = "/com/canonical/qtgmenu";

  QtGMenuImporter m_importer;
  GDBusConnection* m_connection;
  GMenu* m_menu;
};

TEST_F(TestQtGMenu, ExportImportMenu)
{
  QSignalSpy items_changed_spy( &m_importer, SIGNAL( MenuItemsChanged() ) );
  QSignalSpy menu_appeared_spy( &m_importer, SIGNAL( MenuAppeared() ) );
  QSignalSpy menu_disappeared_spy( &m_importer, SIGNAL( MenuDisappeared() ) );

  // no menu exported

  g_menu_append( m_menu, "New", "app.new" );

  auto qmenu = m_importer.GetQMenu();
  EXPECT_EQ( nullptr, qmenu );

  // export menu

  guint export_id = g_dbus_connection_export_menu_model( m_connection, c_path,
      G_MENU_MODEL( m_menu ), NULL );

  menu_appeared_spy.wait();

  qmenu = m_importer.GetQMenu();
  EXPECT_NE( nullptr, qmenu );

  int item_count = m_importer.GetItemCount();
  EXPECT_EQ( 1, item_count );

  // add 2 items

  g_menu_append( m_menu, "Add", "app.add" );
  items_changed_spy.wait();

  g_menu_append( m_menu, "Del", "app.del" );
  items_changed_spy.wait();

  item_count = m_importer.GetItemCount();
  EXPECT_EQ( 3, item_count );

  // add 1 items

  g_menu_append( m_menu, "Quit", "app.quit" );
  items_changed_spy.wait();

  item_count = m_importer.GetItemCount();
  EXPECT_EQ( 4, item_count );

  // unexport menu

  g_dbus_connection_unexport_menu_model( m_connection, export_id );

  m_importer.ForceRefresh();
  menu_disappeared_spy.wait();

  qmenu = m_importer.GetQMenu();
  EXPECT_EQ( nullptr, qmenu );

  item_count = m_importer.GetItemCount();
  EXPECT_EQ( 0, item_count );
}

TEST_F(TestQtGMenu, ExportImportActions)
{
}

} // namespace
