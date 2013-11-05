#include <gio/gio.h>
#include <gtest/gtest.h>

#include <libqtgmenu/QtGMenuImporter.h>
#include <QApplication>

using namespace qtgmenu;

namespace
{

gboolean mainloop_timeout (gpointer user_data)
{
    GMainLoop* loop = (GMainLoop*)user_data;
    g_main_loop_quit(loop);
    return FALSE;
}

void mainloop (guint timeout)
{
    GMainLoop* mainloop = g_main_loop_new (NULL, FALSE);
    g_timeout_add (timeout, mainloop_timeout, mainloop);
    g_main_loop_run (mainloop);
    g_main_loop_unref(mainloop);
}

TEST(TestQtGMenu, ExportImportMenu)
{
    int argc = 0;
    QApplication a(argc, nullptr);

    g_bus_own_name (G_BUS_TYPE_SESSION, "com.canonical.qtgmenu", G_BUS_NAME_OWNER_FLAGS_NONE,
                    NULL, NULL, NULL, NULL, NULL);

    mainloop(500);

    GDBusConnection* connection = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, NULL);

    GMenu* menu = g_menu_new ();
    g_menu_append (menu, "Add", "app.add");
    g_menu_append (menu, "Del", "app.del");
    g_menu_append (menu, "Quit", "app.quit");

    QtGMenuImporter importer( "com.canonical.qtgmenu", "/com/canonical/qtgmenu" );

    auto qmenu = importer.Menu();
    EXPECT_EQ( nullptr, qmenu );

    guint export_id = g_dbus_connection_export_menu_model (connection, "/com/canonical/qtgmenu", G_MENU_MODEL (menu), NULL);

    qmenu = importer.Menu();
    EXPECT_NE( nullptr, qmenu );

    g_object_unref(menu);
    g_object_unref(connection);
}

} // namespace
