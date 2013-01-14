
#include "hudtestutils.h"
#include "hudstringlist.h"

#include <libdbustest/dbus-test.h>
#include <gio/gio.h>

static void
dbus_mock_method_array_free_func(gpointer data)
{
  g_assert(data != NULL);

  DBusMockMethod *method = (DBusMockMethod *) data;
  g_free(method->name);
  g_free(method->in_sig);
  g_free(method->out_sig);
  g_free(method->code);
  g_free(method);
}

DBusMockProperties*
dbus_mock_new_properties()
{
  return g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
}

DBusMockMethods *
dbus_mock_new_methods()
{
  return g_ptr_array_new_with_free_func(dbus_mock_method_array_free_func);
}

void
dbus_mock_methods_append (DBusMockMethods *methods, const gchar *name,
    const gchar *in_sig, const gchar *out_sig, const gchar *code)
{
  DBusMockMethod *method = g_new(DBusMockMethod, 1);
  method->name = g_strdup(name);
  method->in_sig = g_strdup(in_sig);
  method->out_sig = g_strdup(out_sig);
  method->code = g_strdup(code);
  g_ptr_array_add(methods, method);
}

void
dbus_mock_property_append(DBusMockProperties *properties, const gchar *key, GVariant * value)
{
  g_hash_table_insert(properties, g_strdup(key), value);
}

static void
dbus_mock_for_each_property (gpointer key, gpointer value,
    gpointer user_data)
{
  GVariantBuilder *builder = (GVariantBuilder *) user_data;
  g_variant_builder_add (builder, "{sv}", (const gchar *) key,
      (GVariant *) value);
}

static void
dbus_mock_for_each_method(gpointer item, gpointer user_data)
{
  DBusMockMethod *method = (DBusMockMethod *) item;
  GVariantBuilder *builder = (GVariantBuilder *) user_data;
  g_variant_builder_add (builder, "(ssss)", method->name, method->in_sig,
      method->out_sig, method->code);
}


void
dbus_mock_add_object (GDBusConnection *connection, const gchar* bus_name,
    const gchar* object_path, const gchar *path, const gchar *interface,
    DBusMockProperties *properties, GPtrArray *methods)
{
  GError *error;
  GVariantBuilder *builder;

  /* ssa{sv}a(ssss) path, interface, properties, methods */
  /* methods: name, in_sig, out_sig, code */
  builder = g_variant_builder_new (G_VARIANT_TYPE ("(ssa{sv}a(ssss))"));
  g_variant_builder_add (builder, "s", path);
  g_variant_builder_add (builder, "s", interface);
  g_variant_builder_open(builder, G_VARIANT_TYPE_VARDICT);
  g_hash_table_foreach(properties, dbus_mock_for_each_property, builder);
  g_variant_builder_close(builder);
  g_variant_builder_open(builder, G_VARIANT_TYPE ("a(ssss)"));
  g_ptr_array_foreach(methods, dbus_mock_for_each_method, builder);
  g_variant_builder_close(builder);

  error = NULL;
  g_dbus_connection_call_sync (connection, bus_name,
      object_path, "org.freedesktop.DBus.Mock",
      "AddObject", g_variant_builder_end (builder), NULL,
      G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
  g_variant_builder_unref (builder);
  if (error)
  {
    g_warning("%s %s\n", "The request failed:", error->message);
    g_error_free (error);
  }

  g_hash_table_destroy(properties);
  g_ptr_array_free(methods, TRUE);
}

void
dbus_mock_add_method (GDBusConnection *connection,
    const gchar *bus_name, const gchar *path, const gchar *interface,
    const gchar *name, const gchar *in_sig, const gchar *out_sig,
    const gchar *code)
{
  GError *error;

  /* interface, name, in_sig, out_sig, code */
  error = NULL;
  g_dbus_connection_call_sync (connection, bus_name, path,
      "org.freedesktop.DBus.Mock", "AddMethod",
      g_variant_new ("(sssss)", interface, name, in_sig, out_sig, code), NULL,
      G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
  if (error)
  {
    g_warning("%s %s\n", "The request failed:", error->message);
    g_error_free (error);
  }
}

/* If we can't get the name, we should error the test */
gboolean
hud_test_utils_name_timeout (gpointer user_data)
{
  g_error("Unable to get name");
  return FALSE;
}

void
hud_test_utils_start_python_dbusmock (DbusTestService **service,
    GDBusConnection **session, const gchar *name, const gchar *path,
    const gchar *interface)
{
  *service = dbus_test_service_new(NULL);

  /* DBus Mock Process */
  DbusTestProcess * dbusmock = dbus_test_process_new("python3");
  dbus_test_process_append_param(dbusmock, "-m");
  dbus_test_process_append_param(dbusmock, "dbusmock");
  dbus_test_process_append_param(dbusmock, name);
  dbus_test_process_append_param(dbusmock, path);
  dbus_test_process_append_param(dbusmock, interface);
  dbus_test_task_set_name(DBUS_TEST_TASK(dbusmock), "DBus Mock");
  dbus_test_service_add_task(*service, DBUS_TEST_TASK(dbusmock));
  g_object_unref(dbusmock);

  /* Dummy */
  DbusTestTask * dummy = dbus_test_task_new();
  dbus_test_task_set_wait_for(dummy, name);
  dbus_test_service_add_task(*service, dummy);
  g_object_unref(dummy);

  /* Setup timeout */
  guint timeout_source = g_timeout_add_seconds(2, hud_test_utils_name_timeout, NULL);

  /* Get loader up and running and us on that bus */
  g_debug("Starting up DBus Mock");
  dbus_test_service_start_tasks(*service);

  /* Cleanup timeout */
  g_source_remove(timeout_source);

  /* Set us not to exit when the service goes */
  *session = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, NULL);
  g_dbus_connection_set_exit_on_close(*session, FALSE);

  g_debug("Unique connection name: [%s]", g_dbus_connection_get_unique_name(*session));
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
