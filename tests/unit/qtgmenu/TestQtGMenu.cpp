#include <gio/gio.h>
#include <gtest/gtest.h>

namespace
{

gboolean mainloop_timeout (gpointer user_data)
{
    GMainLoop* loop = (GMainLoop*)user_data;
    g_main_loop_quit(loop);
    return FALSE;
}

static void items_changed_event (GMenuModel* model, gint position, gint removed, gint added, gpointer user_data)
{
    std::cout << "items changed";
}

TEST(TestQtGMenu, ExportImportMenu)
{
    GMainLoop* mainloop = g_main_loop_new (NULL, FALSE);

    g_bus_own_name (G_BUS_TYPE_SESSION, "com.canonical.qtgmenu", G_BUS_NAME_OWNER_FLAGS_NONE,
                    NULL, NULL, NULL, NULL, NULL);

    g_timeout_add (500, mainloop_timeout, mainloop);
    g_main_loop_run (mainloop);

    GDBusConnection* connection = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, NULL);

    GMenu* menu = g_menu_new ();
    g_menu_append (menu, "Add", "app.add");
    g_menu_append (menu, "Del", "app.del");
    g_menu_append (menu, "Quit", "app.quit");

    guint export_id = g_dbus_connection_export_menu_model (connection, "/com/canonical/qtgmenu", G_MENU_MODEL (menu), NULL);

    g_timeout_add (500, mainloop_timeout, mainloop);
    g_main_loop_run (mainloop);

    GMenuModel* model = G_MENU_MODEL( g_dbus_menu_model_get(
                                          connection,
                                          "com.canonical.qtgmenu",
                                          "/com/canonical/qtgmenu") );

    gboolean is_mutable = g_menu_model_is_mutable(model);

    g_signal_connect(model, "items-changed", G_CALLBACK (items_changed_event), NULL);

    g_timeout_add (2000, mainloop_timeout, mainloop);
    g_main_loop_run (mainloop);

    gint item_count = g_menu_model_get_n_items(model);

    g_object_unref(model);
    g_object_unref(menu);
    g_object_unref(connection);
    g_main_loop_unref(mainloop);
}

} // namespace
