
#include "hudtestutils.h"
#include "hudstringlist.h"

#include <libdbustest/dbus-test.h>
#include <gio/gio.h>

/* If we can't get the name, we should error the test */
gboolean
hud_test_utils_name_timeout (gpointer user_data)
{
  g_error("Unable to get name");
  return FALSE;
}

/* Start things up with a basic mock-json-app and wait until it starts */
void
hud_test_utils_start_dbusmenu_mock_app (DbusTestService ** service, GDBusConnection ** session, const gchar * jsonfile)
{
  *service = dbus_test_service_new(NULL);

  /* Loader */
  DbusTestProcess * loader = dbus_test_process_new(DBUSMENU_JSON_LOADER);
  dbus_test_process_append_param(loader, HUD_TEST_UTILS_LOADER_NAME);
  dbus_test_process_append_param(loader, HUD_TEST_UTILS_LOADER_PATH);
  dbus_test_process_append_param(loader, jsonfile);
  dbus_test_task_set_name(DBUS_TEST_TASK(loader), "JSON Loader");
  dbus_test_service_add_task(*service, DBUS_TEST_TASK(loader));
  g_object_unref(loader);

  /* Dummy */
  DbusTestTask * dummy = dbus_test_task_new();
  dbus_test_task_set_wait_for(dummy, HUD_TEST_UTILS_LOADER_NAME);
  dbus_test_service_add_task(*service, dummy);
  g_object_unref(dummy);

  /* Setup timeout */
  guint timeout_source = g_timeout_add_seconds(2, hud_test_utils_name_timeout, NULL);

  /* Get loader up and running and us on that bus */
  g_debug("Starting up Dbusmenu Loader");
  dbus_test_service_start_tasks(*service);

  /* Cleanup timeout */
  g_source_remove(timeout_source);

  /* Set us not to exit when the service goes */
  *session = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, NULL);
  g_dbus_connection_set_exit_on_close(*session, FALSE);
}

/* Start things up with a basic mock-json-app and wait until it starts */
void
hud_test_utils_start_model_mock_app (DbusTestService ** service, GDBusConnection ** session, const gchar * appname)
{
  *service = dbus_test_service_new(NULL);

  /* Loader */
  DbusTestProcess * loader = dbus_test_process_new(appname);
  dbus_test_process_append_param(loader, HUD_TEST_UTILS_LOADER_NAME);
  dbus_test_process_append_param(loader, HUD_TEST_UTILS_LOADER_PATH);
  dbus_test_task_set_name(DBUS_TEST_TASK(loader), "Mock Model");
  dbus_test_service_add_task(*service, DBUS_TEST_TASK(loader));
  g_object_unref(loader);

  /* Dummy */
  DbusTestTask * dummy = dbus_test_task_new();
  dbus_test_task_set_wait_for(dummy, HUD_TEST_UTILS_LOADER_NAME);
  dbus_test_service_add_task(*service, dummy);
  g_object_unref(dummy);

  /* Setup timeout */
  guint timeout_source = g_timeout_add_seconds(2, hud_test_utils_name_timeout, NULL);

  /* Get mock up and running and us on that bus */
  g_debug("Starting up Model Mock");
  dbus_test_service_start_tasks(*service);

  /* Cleanup timeout */
  g_source_remove(timeout_source);

  /* Set us not to exit when the service goes */
  *session = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, NULL);
  g_dbus_connection_set_exit_on_close(*session, FALSE);

  return;
}

/* Timeout on our loop */
gboolean
hud_test_utils_timeout_quit_func (gpointer user_data)
{
  GMainLoop * loop = (GMainLoop *)user_data;
  g_main_loop_quit(loop);
  return FALSE;
}

void
hud_test_utils_process_mainloop (const guint delay)
{
  GMainLoop * temploop = g_main_loop_new (NULL, FALSE);
  g_timeout_add (delay, hud_test_utils_timeout_quit_func, temploop);
  g_main_loop_run (temploop);
  g_main_loop_unref (temploop);
}
