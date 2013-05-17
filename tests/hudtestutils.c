
#include "hudtestutils.h"
#include "hudstringlist.h"
#include "hudresult.h"
#include "shared-values.h"

#include <libdbustest/dbus-test.h>
#include <gio/gio.h>
#include <stdarg.h>

/* The max amount of time we should wait for the session bus to shutdown all
 of its objects.  It's in another thread so it's hard to be deterministic
 about when it should happen, but we need things cleaned up. */
#define SESSION_MAX_WAIT 20

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
    DBusMockProperties *properties, DBusMockMethods *methods)
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

void
dbus_mock_get_method_calls (GDBusConnection *connection,
    const gchar *bus_name, const gchar *path, const gchar *method_name,
    GVariant **response)
{
  GError *error;

  /* interface, name, in_sig, out_sig, code */
  error = NULL;
  *response = g_dbus_connection_call_sync (connection, bus_name, path,
      "org.freedesktop.DBus.Mock", "GetMethodCalls",
      g_variant_new ("(s)", method_name), G_VARIANT_TYPE("(a(tav))"),
      G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
  if (error)
  {
    g_warning("%s %s\n", "The request failed:", error->message);
    g_error_free (error);
  }
}

void
dbus_mock_clear_method_calls (GDBusConnection *connection,
    const gchar *bus_name, const gchar *path)
{
  GError *error;

  /* interface, name, in_sig, out_sig, code */
  error = NULL;
  g_dbus_connection_call_sync (connection, bus_name, path,
      "org.freedesktop.DBus.Mock", "ClearCalls", NULL, NULL,
      G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
  if (error)
  {
    g_warning("%s %s\n", "The request failed:", error->message);
    g_error_free (error);
  }
}

void
dbus_mock_assert_method_call_results (GDBusConnection *connection,
    const gchar *name, const gchar *path, const gchar* method_name,
    const gchar *regex)
{
  GVariant *response = NULL;
  dbus_mock_get_method_calls (connection, name, path, method_name, &response);
  gchar *response_string = g_variant_print (response, FALSE);
  g_debug("Response: [%s]", response_string);
  g_assert( g_regex_match_simple(regex, response_string, 0, 0));
  g_free (response_string);
  g_variant_unref (response);
}

DBusMockSignalArgs *
dbus_mock_new_signal_args()
{
  return g_ptr_array_new_with_free_func(NULL);
}

void
dbus_mock_signal_args_append(DBusMockSignalArgs* args, GVariant * value)
{
  g_ptr_array_add(args, value);
}

static void
dbus_mock_for_each_signal_arg(gpointer item, gpointer user_data)
{
  GVariant *value = (GVariant *) item;
  GVariantBuilder *builder = (GVariantBuilder *) user_data;
  g_variant_builder_add(builder, "v", value);
}

void
dbus_mock_emit_signal (GDBusConnection *connection,
    const gchar *bus_name, const gchar *path, const gchar *interface,
    const gchar *signal_name, const gchar *signature, DBusMockSignalArgs *args)
{
  GError *error;
  GVariantBuilder *builder;

  /* interface, name, signature, args */
  builder = g_variant_builder_new (G_VARIANT_TYPE ("(sssav)"));
  g_variant_builder_add (builder, "s", interface);
  g_variant_builder_add (builder, "s", signal_name);
  g_variant_builder_add (builder, "s", signature);
  g_variant_builder_open(builder, G_VARIANT_TYPE ("av"));
  g_ptr_array_foreach(args, dbus_mock_for_each_signal_arg, builder);
  g_variant_builder_close(builder);

  error = NULL;
  g_dbus_connection_call_sync (connection, bus_name, path,
      "org.freedesktop.DBus.Mock", "EmitSignal",
      g_variant_builder_end(builder), NULL,
      G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
  g_variant_builder_unref (builder);
  g_ptr_array_free(args, TRUE);
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
hud_test_utils_dbus_mock_start (DbusTestService* service,
    const gchar* name, const gchar* path, const gchar* interface)
{
  /* DBus Mock Process */
  DbusTestProcess* dbusmock = dbus_test_process_new ("python3");
  dbus_test_process_append_param (dbusmock, "-m");
  dbus_test_process_append_param (dbusmock, "dbusmock");
  dbus_test_process_append_param (dbusmock, name);
  dbus_test_process_append_param (dbusmock, path);
  dbus_test_process_append_param (dbusmock, interface);
  dbus_test_task_set_name (DBUS_TEST_TASK(dbusmock), "DBus Mock");
  dbus_test_service_add_task (service, DBUS_TEST_TASK(dbusmock) );
  g_object_unref (dbusmock);
}

GDBusConnection *
hud_test_utils_mock_dbus_connection_new(DbusTestService *service, const gchar *name, ...)
{
  va_list ap;

  va_start (ap, name);
  while(name)
  {
    g_debug("Waiting for task [%s]", name);
    DbusTestTask* dummy = dbus_test_task_new ();
    dbus_test_task_set_wait_for (dummy, name);
    name = va_arg(ap, const gchar *);
    dbus_test_service_add_task (service, dummy);
    g_object_unref (dummy);
  }
  va_end (ap);

  /* Setup timeout */
  guint timeout_source = g_timeout_add_seconds (5, hud_test_utils_name_timeout,
      NULL );

  /* Get loader up and running and us on that bus */
  g_debug("Starting up DBus test service tasks");
  dbus_test_service_start_tasks (service);

  /* Cleanup timeout */
  g_source_remove (timeout_source);

  /* Set us not to exit when the service goes */
  GDBusConnection* session = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, NULL );
  g_dbus_connection_set_exit_on_close (session, FALSE);

  return session;
}

void
hud_test_utils_json_loader_start_full (DbusTestService *service,
  const gchar *name, const gchar *path, const gchar *jsonfile)
{
  DbusTestProcess * loader = dbus_test_process_new(DBUSMENU_JSON_LOADER);
  dbus_test_process_append_param(loader, name);
  dbus_test_process_append_param(loader, path);
  dbus_test_process_append_param(loader, jsonfile);
  dbus_test_task_set_name(DBUS_TEST_TASK(loader), "JSON Loader");
  dbus_test_service_add_task(service, DBUS_TEST_TASK(loader));
  g_object_unref(loader);
}

void
hud_test_utils_json_loader_start (DbusTestService *service, const gchar *jsonfile)
{
  hud_test_utils_json_loader_start_full (service, HUD_TEST_UTILS_LOADER_NAME,
      HUD_TEST_UTILS_LOADER_PATH, jsonfile);
}

/* Start things up with a basic mock-json-app and wait until it starts */
void
hud_test_utils_start_dbusmenu_mock_app (DbusTestService ** service, GDBusConnection ** session, const gchar * jsonfile)
{
  *service = dbus_test_service_new (NULL);

  hud_test_utils_json_loader_start (*service, jsonfile);

  *session = hud_test_utils_mock_dbus_connection_new (*service,
      HUD_TEST_UTILS_LOADER_NAME, NULL);
}

void
hud_test_utils_start_menu_model (DbusTestService* service,
  const gchar* appname)
{
  hud_test_utils_start_menu_model_full (service, appname,
      HUD_TEST_UTILS_LOADER_NAME, HUD_TEST_UTILS_LOADER_PATH,
      FALSE);
}

void
hud_test_utils_start_menu_model_full (DbusTestService* service,
  const gchar* appname, const gchar *name, const gchar *path,
  const gboolean is_application)
{
  DbusTestProcess* loader = dbus_test_process_new (appname);
  dbus_test_process_append_param (loader, name);
  dbus_test_process_append_param (loader, path);
  if (is_application)
  {
    dbus_test_process_append_param (loader, "TRUE");
  }
  dbus_test_task_set_name (DBUS_TEST_TASK(loader), "Mock Model");
  dbus_test_service_add_task (service, DBUS_TEST_TASK(loader) );
  g_object_unref (loader);
}

/* Start things up with a basic mock-json-app and wait until it starts */
void
hud_test_utils_start_model_mock_app (DbusTestService ** service, GDBusConnection ** session, const gchar * appname)
{
  *service = dbus_test_service_new(NULL);

  hud_test_utils_start_menu_model (*service, appname);

  *session = hud_test_utils_mock_dbus_connection_new (*service,
        HUD_TEST_UTILS_LOADER_NAME, NULL);
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


/*
 * Waiting until the session bus shuts down
 */
void
hud_test_utils_wait_for_connection_close (GDBusConnection *connection)
{
  g_object_add_weak_pointer(G_OBJECT(connection), (gpointer) &connection);

  g_object_unref (connection);

  int wait_count;
  for (wait_count = 0; connection != NULL && wait_count < SESSION_MAX_WAIT;
      wait_count++)
  {
    hud_test_utils_process_mainloop (200);
  }

  g_assert(wait_count != SESSION_MAX_WAIT);
}

void
hud_test_utils_results_append_func(HudResult *result, gpointer user_data)
{
  g_assert(result != NULL);
  g_assert(HUD_IS_RESULT(result));

  g_assert(user_data != NULL);
  GPtrArray *results = (GPtrArray *) user_data;

  g_ptr_array_add(results, result);
}

gint
hud_test_utils_results_compare_func(gconstpointer a, gconstpointer b)
{
  return hud_result_get_distance (*(HudResult **) a, 0)
        - hud_result_get_distance (*(HudResult **) b, 0);
}

void
hud_test_utils_source_assert_result (GPtrArray* results, const guint index, const gchar* value)
{
  HudResult *result = HUD_RESULT(g_ptr_array_index(results, index));

  HudItem *item = hud_result_get_item (result);
  g_assert(item != NULL);
  g_assert(HUD_IS_ITEM(item));

  HudStringList *tokens = hud_item_get_tokens (item);
  g_assert_cmpstr(hud_string_list_get_head(tokens), ==, value);
}

static gboolean hud_test_utils_ignore_dbus_null_connection_callback (
    const gchar *log_domain, GLogLevelFlags level, const gchar *message,
    gpointer user_data)
{
  if (g_strcmp0 (log_domain, "GLib-GIO") == 0
      && g_str_has_prefix (message,
          "g_dbus_connection_call_finish_internal: assertion 'G_IS_DBUS_CONNECTION"))
  {
    return FALSE;
  }

  if (g_strcmp0 (log_domain, "GLib-GIO") == 0
      && g_str_has_prefix (message,
          "g_dbus_connection_call_finish_internal: assertion `G_IS_DBUS_CONNECTION"))
  {
    return FALSE;
  }

  return TRUE;
}

void
hud_test_utils_ignore_dbus_null_connection()
{
  g_test_log_set_fatal_handler(hud_test_utils_ignore_dbus_null_connection_callback, NULL);
}

const gchar *QUERY_PATH = "/com/canonical/hud/query0";

const gchar * results_model_schema[] = {
  "v", /* Command ID */
  "s", /* Command Name */
  "a(ii)", /* Highlights in command name */
  "s", /* Description */
  "a(ii)", /* Highlights in description */
  "s", /* Shortcut */
  "u", /* Distance */
  "b", /* Parameterized */
};

const gchar * appstack_model_schema[] = {
  "s", /* Application ID */
  "s", /* Icon Name */
  "i", /* Item Type */
};

void
hud_test_utils_add_result (DeeModel *results_model, guint64 id,
    const gchar *command, const gchar *description, const gchar *shortcut,
    guint32 distance, gboolean parameterized)
{
  GVariant * columns[G_N_ELEMENTS(results_model_schema) + 1];
  columns[0] = g_variant_new_variant(g_variant_new_uint64(id));
  columns[1] = g_variant_new_string(command);
  columns[2] = g_variant_new_array(G_VARIANT_TYPE("(ii)"), NULL, 0);
  columns[3] = g_variant_new_string(description);
  columns[4] = g_variant_new_array(G_VARIANT_TYPE("(ii)"), NULL, 0);
  columns[5] = g_variant_new_string(shortcut);
  columns[6] = g_variant_new_uint32(distance);
  columns[7] = g_variant_new_boolean(parameterized);
  columns[8] = NULL;

  dee_model_append_row (results_model, columns);
}

void
hud_test_utils_start_hud_service (DbusTestService **service,
    GDBusConnection **connection, DeeModel **results_model,
    DeeModel **appstack_model)
{
  *service = dbus_test_service_new (NULL );
  hud_test_utils_dbus_mock_start (*service, DBUS_NAME, DBUS_PATH, DBUS_IFACE);
  *connection = hud_test_utils_mock_dbus_connection_new (*service, DBUS_NAME,
      NULL );
  hud_test_utils_process_mainloop (300);

  {
    DBusMockProperties* properties = dbus_mock_new_properties ();
    dbus_mock_property_append (properties, "ResultsModel",
        g_variant_new_string ("com.canonical.hud.query0.results"));
    dbus_mock_property_append (properties, "AppstackModel",
        g_variant_new_string ("com.canonical.hud.query0.appstack"));
    dbus_mock_property_append (properties, "ToolbarItems",
        g_variant_new_array (G_VARIANT_TYPE_STRING, NULL, 0));
    DBusMockMethods* methods = dbus_mock_new_methods ();
    dbus_mock_methods_append (methods, "UpdateQuery", "s", "i", "ret = 1");
    dbus_mock_methods_append (methods, "VoiceQuery", "", "is", "ret = (1, 'voice query')");
    dbus_mock_methods_append (methods, "UpdateApp", "s", "i", "ret = 1");
    dbus_mock_methods_append (methods, "CloseQuery", "", "", "");
    dbus_mock_methods_append (methods, "ExecuteCommand", "vu", "", "");
    dbus_mock_methods_append (methods, "ExecuteParameterized", "vu", "sooi", "ret = ('action', '/action/path', '/model/path', 1)");
    dbus_mock_methods_append (methods, "ExecuteToolbar", "su", "", "");
    dbus_mock_add_object (*connection, DBUS_NAME, DBUS_PATH, QUERY_PATH,
        "com.canonical.hud.query", properties, methods);
  }

  *results_model = dee_shared_model_new("com.canonical.hud.query0.results");
  dee_model_set_schema_full(*results_model, results_model_schema, G_N_ELEMENTS(results_model_schema));

  *appstack_model = dee_shared_model_new("com.canonical.hud.query0.appstack");
  dee_model_set_schema_full(*appstack_model, appstack_model_schema, G_N_ELEMENTS(appstack_model_schema));

  /* query */
  dbus_mock_add_method (*connection, DBUS_NAME, DBUS_PATH, DBUS_IFACE,
      "CreateQuery", "s", "ossi",
      "ret = ('/com/canonical/hud/query0', 'com.canonical.hud.query0.results', 'com.canonical.hud.query0.appstack', dbus.Int32(0))");

  /* id */
  dbus_mock_add_method (*connection, DBUS_NAME, DBUS_PATH, DBUS_IFACE,
      "RegisterApplication", "s", "o",
      "ret = ('/app/object')");

  hud_test_utils_process_mainloop (100);
}

void
hud_test_utils_stop_hud_service (DbusTestService *service,
    GDBusConnection *connection, DeeModel *results_model,
    DeeModel *appstack_model)
{
  g_object_unref(results_model);
  g_object_unref(appstack_model);
  g_object_unref(service);
  hud_test_utils_wait_for_connection_close(connection);
}

gboolean
no_dee_add_match (const gchar * log_domain, GLogLevelFlags level,
    const gchar * message, gpointer user_data)
{
  if (g_strcmp0(log_domain, "GLib-GIO") == 0 &&
      g_str_has_prefix(message, "Error while sending AddMatch()")) {
    return FALSE;
  }

  if (g_strcmp0(log_domain, "GLib-GIO") == 0 &&
      g_str_has_prefix(message, "g_dbus_connection_call_finish_internal: assertion 'G_IS_DBUS_CONNECTION")) {
    return FALSE;
  }

  if (g_strcmp0(log_domain, "GLib-GIO") == 0 &&
      g_str_has_prefix(message, "g_dbus_connection_call_finish_internal: assertion `G_IS_DBUS_CONNECTION")) {
    return FALSE;
  }

  return TRUE;
}
