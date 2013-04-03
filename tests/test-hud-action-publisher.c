
#include <hud.h>
#include <libdbustest/dbus-test.h>

#include "shared-values.h"
#include "hudtestutils.h"

static void
test_action_publisher_new_for_id ()
{
  DbusTestService *service = NULL;
  GDBusConnection *connection = NULL;
  DeeModel *results_model = NULL;
  DeeModel *appstack_model = NULL;

  hud_test_utils_start_hud_service (&service, &connection, &results_model,
      &appstack_model);

  {
    HudActionPublisher *publisher = hud_action_publisher_new_for_id(NULL);
    g_assert(publisher);
    g_object_unref (publisher);
  }

  {
    HudActionPublisher *publisher = hud_action_publisher_new_for_id (
        g_variant_new_int32 (123));
    g_assert(publisher);
    g_object_unref (publisher);
  }

  hud_test_utils_stop_hud_service (service, connection, results_model,
      appstack_model);
}

static void
test_action_publisher_new_for_application ()
{
  DbusTestService *service = NULL;
  GDBusConnection *connection = NULL;
  DeeModel *results_model = NULL;
  DeeModel *appstack_model = NULL;

  hud_test_utils_start_hud_service (&service, &connection, &results_model,
      &appstack_model);

  GApplication *application = g_application_new("app.id", G_APPLICATION_FLAGS_NONE);
  GError *error = NULL;
  if (!g_application_register(application, NULL, &error))
  {
    g_error("%s", error->message);
  }

  HudActionPublisher *publisher = hud_action_publisher_new_for_application(application);
  g_assert(publisher);

  g_object_unref (application);
  g_object_unref (publisher);

  hud_test_utils_stop_hud_service (service, connection, results_model,
      appstack_model);
}

static void
test_suite (void)
{
  g_test_add_func ("/hud/hud/action-publisher/new_for_id", test_action_publisher_new_for_id);
  g_test_add_func ("/hud/hud/action-publisher/new_for_application", test_action_publisher_new_for_application);
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
