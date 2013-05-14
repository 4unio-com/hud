
#include <hud.h>
#include <libdbustest/dbus-test.h>

#include "shared-values.h"
#include "hudtestutils.h"

static void
test_manager_create ()
{
  DbusTestService *service = NULL;
  GDBusConnection *connection = NULL;
  DeeModel *results_model = NULL;
  DeeModel *appstack_model = NULL;

  hud_test_utils_start_hud_service (&service, &connection, &results_model,
      &appstack_model);

  HudManager *manager = hud_manager_new ("test.app");
  hud_test_utils_process_mainloop (100);

  dbus_mock_assert_method_call_results (connection, DBUS_NAME, DBUS_PATH,
      "RegisterApplication",
      "\\(\\[\\(\\d+, \\[<'test.app'>\\]\\)\\],\\)");

  g_object_unref (manager);

  hud_test_utils_stop_hud_service (service, connection, results_model,
      appstack_model);
}

static void
test_manager_create_with_application ()
{
  DbusTestService *service = NULL;
  GDBusConnection *connection = NULL;
  DeeModel *results_model = NULL;
  DeeModel *appstack_model = NULL;

  hud_test_utils_start_hud_service (&service, &connection, &results_model,
      &appstack_model);

  {
    DBusMockProperties* properties = dbus_mock_new_properties ();
    DBusMockMethods* methods = dbus_mock_new_methods ();
    dbus_mock_methods_append (methods, "AddSources", "a(usso)a(uso)", "", "");
    dbus_mock_add_object (connection, DBUS_NAME, DBUS_PATH, "/app/object",
        "com.canonical.hud.Application", properties, methods);
  }

  GApplication *application = g_application_new("app.id", G_APPLICATION_FLAGS_NONE);
  GError *error = NULL;
  if (!g_application_register(application, NULL, &error))
  {
    g_error("%s", error->message);
  }

  HudManager *manager = hud_manager_new_for_application(application);
  hud_test_utils_process_mainloop (100);
  g_object_unref (manager);

  dbus_mock_assert_method_call_results (connection, DBUS_NAME, DBUS_PATH,
      "RegisterApplication",
      "\\(\\[\\(\\d+, \\[<'app.id'>\\]\\)\\],\\)");

  dbus_mock_assert_method_call_results (connection, DBUS_NAME, "/app/object",
      "AddSources",
      "\\(\\[\\(\\d+, \\[<\\[\\(uint32 0, 'action-publisher-context-0', 'app', objectpath '\\/app\\/id'\\)\\]>, <\\[\\(uint32 0, 'action-publisher-context-0', objectpath '\\/com\\/canonical\\/hud\\/publisher0'\\)\\]>\\]\\)\\],\\)");

  g_object_unref (application);

  hud_test_utils_stop_hud_service (service, connection, results_model,
      appstack_model);
}

static void
test_manager_add_actions ()
{
  DbusTestService *service = NULL;
  GDBusConnection *connection = NULL;
  DeeModel *results_model = NULL;
  DeeModel *appstack_model = NULL;

  hud_test_utils_start_hud_service (&service, &connection, &results_model,
      &appstack_model);

  {
    DBusMockProperties* properties = dbus_mock_new_properties ();
    DBusMockMethods* methods = dbus_mock_new_methods ();
    dbus_mock_methods_append (methods, "AddSources", "a(usso)a(uso)", "", "");
    dbus_mock_add_object (connection, DBUS_NAME, DBUS_PATH, "/app/object",
        "com.canonical.hud.Application", properties, methods);
  }

  HudManager *manager = hud_manager_new ("test.app");
  hud_test_utils_process_mainloop (100);

  HudActionPublisher *publisher = hud_action_publisher_new(HUD_ACTION_PUBLISHER_ALL_WINDOWS, "test-add-context");
  hud_action_publisher_add_action_group(publisher, "app", "/app/object");
  hud_manager_add_actions(manager, publisher);
  hud_test_utils_process_mainloop (100);

  dbus_mock_assert_method_call_results (connection, DBUS_NAME, "/app/object",
        "AddSources",
        "\\(\\[\\(\\d+, \\[<\\[\\(uint32 0, 'test-add-context', 'app', objectpath '\\/app\\/object'\\)\\]>, <\\[\\(uint32 0, 'test-add-context', objectpath '\\/com\\/canonical\\/hud\\/publisher'\\)\\]>\\]\\)\\],\\)");

  g_object_unref (publisher);
  g_object_unref (manager);

  hud_test_utils_stop_hud_service (service, connection, results_model,
      appstack_model);
}

/**
 * the remove_actions method does nothing at the moment
 */

static void
test_manager_remove_actions ()
{
  DbusTestService *service = NULL;
  GDBusConnection *connection = NULL;
  DeeModel *results_model = NULL;
  DeeModel *appstack_model = NULL;

  hud_test_utils_start_hud_service (&service, &connection, &results_model,
      &appstack_model);

  {
    DBusMockProperties* properties = dbus_mock_new_properties ();
    DBusMockMethods* methods = dbus_mock_new_methods ();
    dbus_mock_methods_append (methods, "AddSources", "a(usso)a(uso)", "", "");
    dbus_mock_add_object (connection, DBUS_NAME, DBUS_PATH, "/app/object",
        "com.canonical.hud.Application", properties, methods);
  }

  HudManager *manager = hud_manager_new ("app-id");
  hud_test_utils_process_mainloop (100);

  HudActionPublisher *publisher = hud_action_publisher_new(HUD_ACTION_PUBLISHER_ALL_WINDOWS, "test-context");
  hud_action_publisher_add_action_group(publisher, "app", "/app/object");
  hud_manager_add_actions(manager, publisher);
  hud_test_utils_process_mainloop (100);

  dbus_mock_assert_method_call_results (connection, DBUS_NAME, "/app/object",
        "AddSources",
        "\\(\\[\\(\\d+, \\[<\\[\\(uint32 0, 'test-context', 'app', objectpath '\\/app\\/object'\\)\\]>, <\\[\\(uint32 0, 'test-context', objectpath '\\/com\\/canonical\\/hud\\/publisher'\\)\\]>\\]\\)\\],\\)");
  dbus_mock_clear_method_calls(connection, DBUS_NAME, "/app/object");

  hud_manager_remove_actions(manager, publisher);
  hud_test_utils_process_mainloop (100);

  g_object_unref (publisher);
  g_object_unref (manager);

  hud_test_utils_stop_hud_service (service, connection, results_model,
      appstack_model);
}

static void
test_suite (void)
{
  g_test_add_func ("/hud/hud/manager/create", test_manager_create);
  g_test_add_func ("/hud/hud/manager/create_with_application", test_manager_create_with_application);
  g_test_add_func ("/hud/hud/manager/add_actions", test_manager_add_actions);
  g_test_add_func ("/hud/hud/manager/remove_actions", test_manager_remove_actions);
}

int
main (int argc, char * argv[])
{
#ifndef GLIB_VERSION_2_36
  g_type_init ();
#endif

  g_test_init(&argc, &argv, NULL);

  /* Test Suites */
  test_suite();

  return g_test_run();
}
