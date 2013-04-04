
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

    g_assert_cmpint(g_variant_get_int32(hud_action_publisher_get_id(publisher)),
        ==, -1);

    g_object_unref (publisher);
  }

  {
    HudActionPublisher *publisher = hud_action_publisher_new_for_id (
        g_variant_new_int32 (123));
    g_assert(publisher);

    g_assert_cmpint(g_variant_get_int32(hud_action_publisher_get_id(publisher)),
        ==, 123);

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

  g_assert(!hud_action_publisher_get_id(publisher));

  g_object_unref (application);
  g_object_unref (publisher);

  hud_test_utils_stop_hud_service (service, connection, results_model,
      appstack_model);
}

static void
test_action_publisher_add_action_group ()
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

  {
    GList* groups = hud_action_publisher_get_action_groups (publisher);
    g_assert_cmpuint(g_list_length(groups), ==, 1);
    {
      HudActionPublisherActionGroupSet *group =
          (HudActionPublisherActionGroupSet *) g_list_nth_data (groups, 0);
      g_assert_cmpstr(group->path, ==, "/app/id");
      g_assert_cmpstr(group->prefix, ==, "app");
    }
  }

  hud_action_publisher_add_action_group(publisher, "prefix", "/object/path");

  {
    GList* groups = hud_action_publisher_get_action_groups (publisher);
    g_assert_cmpuint(g_list_length(groups), ==, 2);
    {
      HudActionPublisherActionGroupSet *group =
          (HudActionPublisherActionGroupSet *) g_list_nth_data (groups, 0);
      g_assert_cmpstr(group->path, ==, "/object/path");
      g_assert_cmpstr(group->prefix, ==, "prefix");
    }
    {
      HudActionPublisherActionGroupSet *group =
          (HudActionPublisherActionGroupSet *) g_list_nth_data (groups, 1);
      g_assert_cmpstr(group->path, ==, "/app/id");
      g_assert_cmpstr(group->prefix, ==, "app");
    }
  }

  g_object_unref (application);
  g_object_unref (publisher);

  hud_test_utils_stop_hud_service (service, connection, results_model,
      appstack_model);
}

static void
test_action_publisher_add_description ()
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

  HudActionDescription *description = hud_action_description_new (
      "hud.simple-action", g_variant_new_string ("Foo") );
  hud_action_description_set_attribute_value (description,
      G_MENU_ATTRIBUTE_LABEL, g_variant_new_string ("Simple Action"));
  g_assert_cmpstr(hud_action_description_get_action_name(description), ==,
      "hud.simple-action");
  g_assert_cmpstr(
      g_variant_get_string(hud_action_description_get_action_target(description), 0),
      ==, "Foo");

  hud_action_publisher_add_description (publisher, description);

  // FIXME Need to make actual assertions about what the publisher is doing here

  g_object_unref (application);
  g_object_unref (publisher);
  hud_action_description_unref(description);

  hud_test_utils_stop_hud_service (service, connection, results_model,
      appstack_model);
}

static void
test_suite (void)
{
  g_test_add_func ("/hud/hud/action-publisher/new_for_id", test_action_publisher_new_for_id);
  g_test_add_func ("/hud/hud/action-publisher/new_for_application", test_action_publisher_new_for_application);
  g_test_add_func ("/hud/hud/action-publisher/add_action_group", test_action_publisher_add_action_group);
  g_test_add_func ("/hud/hud/action-publisher/add_description", test_action_publisher_add_description);
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
