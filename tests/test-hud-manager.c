
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

  HudManager *manager = hud_manager_new ("test-app");
  hud_test_utils_process_mainloop (300);

  dbus_mock_assert_method_call_results (connection, DBUS_NAME, DBUS_PATH,
      "RegisterApplication",
      "\\(\\[\\(\\d+, \\[<'test-app'>\\]\\)\\],\\)");

  g_object_unref (manager);

  hud_test_utils_stop_hud_service (service, connection, results_model,
      appstack_model);
}

static void
test_suite (void)
{
  g_test_add_func ("/hud/hud/manager/create", test_manager_create);
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
