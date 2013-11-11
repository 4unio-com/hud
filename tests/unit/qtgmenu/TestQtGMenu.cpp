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

  int GetMenuItemCount()
  {
    GMenu* menu = m_importer.GetGMenu();

    if( !menu )
    {
      return 0;
    }

    gint item_count = g_menu_model_get_n_items( G_MENU_MODEL( menu ) );
    g_object_unref( menu );

    return item_count;
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

  EXPECT_EQ( nullptr, m_importer.GetQMenu() );

  // export menu

  guint export_id = g_dbus_connection_export_menu_model( m_connection, c_path,
      G_MENU_MODEL( m_menu ), NULL );

  menu_appeared_spy.wait();

  EXPECT_NE( nullptr, m_importer.GetQMenu() );
  EXPECT_EQ( 1, GetMenuItemCount() );

  // add 2 items

  g_menu_append( m_menu, "Add", "app.add" );
  items_changed_spy.wait();

  g_menu_append( m_menu, "Del", "app.del" );
  items_changed_spy.wait();

  EXPECT_EQ( 3, GetMenuItemCount() );

  // add 1 items

  g_menu_append( m_menu, "Quit", "app.quit" );
  items_changed_spy.wait();

  EXPECT_EQ( 4, GetMenuItemCount() );

  // unexport menu

  g_dbus_connection_unexport_menu_model( m_connection, export_id );

  m_importer.ForceRefresh();
  menu_disappeared_spy.wait();

  EXPECT_EQ( nullptr, m_importer.GetQMenu() );
  EXPECT_EQ( 0, GetMenuItemCount() );
}

TEST_F(TestQtGMenu, ExportImportActions)
{
}

} // namespace
