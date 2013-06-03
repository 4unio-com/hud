
#include <hud.h>
#include <libdbustest/dbus-test.h>

#include "shared-values.h"
#include "test-utils.h"

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
    HudActionPublisher *publisher = hud_action_publisher_new(HUD_ACTION_PUBLISHER_ALL_WINDOWS, HUD_ACTION_PUBLISHER_NO_CONTEXT);
    g_assert(publisher);

    g_assert_cmpint(hud_action_publisher_get_window_id(publisher),
        ==, HUD_ACTION_PUBLISHER_ALL_WINDOWS);
    g_assert(hud_action_publisher_get_context_id(publisher) != NULL);

    g_object_unref (publisher);
  }

  {
    HudActionPublisher *publisher = hud_action_publisher_new(123, HUD_ACTION_PUBLISHER_NO_CONTEXT);
    g_assert(publisher);

    g_assert_cmpint(hud_action_publisher_get_window_id(publisher),
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
  hud_test_utils_process_mainloop (100);

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
  hud_test_utils_process_mainloop (100);

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

    GMenuModel *model = G_MENU_MODEL(g_dbus_menu_model_get (connection, DBUS_NAME,
          "/com/canonical/hud/publisher"));
    g_assert(model);
    g_assert_cmpuint(g_menu_model_get_n_items(model), ==, 0);
    g_object_unref(model);
  }

  // FIXME This API method currently does nothing
  hud_action_publisher_remove_action_group (publisher, "prefix",
      g_variant_new_string ("/object/path"));

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

  hud_action_publisher_add_description (publisher, description);
  hud_test_utils_process_mainloop (100);

  {
    GMenuModel *model = G_MENU_MODEL(g_dbus_menu_model_get (connection, DBUS_NAME,
              "/com/canonical/hud/publisher"));
    g_assert(model);
    g_assert_cmpuint(g_menu_model_get_n_items(model), ==, 0);
    g_object_unref(model);
  }

  HudActionDescription *paramdesc = hud_action_description_new (
      "hud.simple-action", NULL );
  hud_action_description_set_attribute_value (paramdesc, G_MENU_ATTRIBUTE_LABEL,
      g_variant_new_string ("Parameterized Action"));
  hud_action_publisher_add_description (publisher, paramdesc);

  GMenu * menu = g_menu_new ();
  g_menu_append_item (menu, g_menu_item_new ("Item One", "hud.simple-action"));
  g_menu_append_item (menu, g_menu_item_new ("Item Two", "hud.simple-action"));
  g_menu_append_item (menu,
      g_menu_item_new ("Item Three", "hud.simple-action"));
  hud_action_description_set_parameterized(paramdesc, G_MENU_MODEL(menu));

  hud_test_utils_process_mainloop (100);

  {
    GMenuModel *model = G_MENU_MODEL(g_dbus_menu_model_get (connection, DBUS_NAME,
              "/com/canonical/hud/publisher"));
    g_assert(model);
    g_assert_cmpuint(g_menu_model_get_n_items(model), ==, 0);
    g_assert(g_menu_model_is_mutable(model));
    g_object_unref(model);
  }

  g_object_unref (application);
  g_object_unref (publisher);
  hud_action_description_unref(description);

  hud_test_utils_stop_hud_service (service, connection, results_model,
      appstack_model);
}

static void
test_action_description_with_attribute_value()
{
  DbusTestService *service = NULL;
  GDBusConnection *connection = NULL;
  DeeModel *results_model = NULL;
  DeeModel *appstack_model = NULL;
  hud_test_utils_start_hud_service (&service, &connection, &results_model,
        &appstack_model);

  HudActionDescription *description = hud_action_description_new (
        "hud.simple-action", g_variant_new_string ("Foo") );
    hud_action_description_set_attribute_value (description,
        G_MENU_ATTRIBUTE_LABEL, g_variant_new_string ("Simple Action"));
  g_assert_cmpstr(hud_action_description_get_action_name(description), ==,
      "hud.simple-action");
  g_assert_cmpstr(
      g_variant_get_string(hud_action_description_get_action_target(description), 0),
      ==, "Foo");

  hud_test_utils_stop_hud_service (service, connection, results_model,
        appstack_model);
}

static void
test_action_description_with_attribute()
{
  DbusTestService *service = NULL;
  GDBusConnection *connection = NULL;
  DeeModel *results_model = NULL;
  DeeModel *appstack_model = NULL;
  hud_test_utils_start_hud_service (&service, &connection, &results_model,
        &appstack_model);

  HudActionDescription *description = hud_action_description_new (
      "hud.simple-action", g_variant_new_string ("Bar") );
  hud_action_description_set_attribute (description, G_MENU_ATTRIBUTE_LABEL,
      "s", "Simple Action");
  g_assert_cmpstr(hud_action_description_get_action_name(description), ==,
      "hud.simple-action");
  g_assert_cmpstr(
      g_variant_get_string(hud_action_description_get_action_target(description), 0),
      ==, "Bar");

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
  g_test_add_func ("/hud/hud/action-publisher/action_description_with_attribute_value", test_action_description_with_attribute_value);
  g_test_add_func ("/hud/hud/action-publisher/action_description_with_attribute ", test_action_description_with_attribute);
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
