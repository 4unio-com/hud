#ifndef HUDTESTUTILS_H_
#define HUDTESTUTILS_H_

#include <glib.h>

typedef struct _HudStringList HudStringList;
typedef struct _GDBusConnection GDBusConnection;
typedef struct _DbusTestService DbusTestService;
typedef struct _HudResult HudResult;

#define HUD_TEST_UTILS_LOADER_NAME "test.json.loader"
#define HUD_TEST_UTILS_LOADER_PATH "/test/json/loader"

typedef struct
{
  gchar *name;
  gchar *in_sig;
  gchar *out_sig;
  gchar *code;
} DBusMockMethod;

typedef GHashTable DBusMockProperties;
typedef GPtrArray DBusMockMethods;
typedef GPtrArray DBusMockSignalArgs;

DBusMockProperties * dbus_mock_new_properties();

DBusMockMethods * dbus_mock_new_methods();

DBusMockSignalArgs * dbus_mock_new_signal_args();

void dbus_mock_signal_args_append (DBusMockSignalArgs* args, GVariant * value);

void dbus_mock_methods_append (DBusMockMethods *methods, const gchar *name,
    const gchar *in_sig, const gchar *out_sig, const gchar *code);

void dbus_mock_property_append (DBusMockProperties *properties,
    const gchar *key, GVariant * value);

void dbus_mock_add_object (GDBusConnection *connection, const gchar* bus_name,
    const gchar* object_path, const gchar *path, const gchar *interface,
    DBusMockProperties *properties, DBusMockMethods *methods);

void dbus_mock_add_method (GDBusConnection *connection, const gchar *bus_name,
    const gchar *path, const gchar *interface, const gchar *name,
    const gchar *in_sig, const gchar *out_sig, const gchar *code);

void dbus_mock_emit_signal (GDBusConnection *connection,
    const gchar *bus_name, const gchar *path, const gchar *interface,
    const gchar *signal_name, const gchar *signature, DBusMockSignalArgs *args);



gboolean hud_test_utils_name_timeout (gpointer user_data);

GDBusConnection * hud_test_utils_mock_dbus_connection_new (
    DbusTestService *service, const gchar *name, ...);

void hud_test_utils_dbus_mock_start (DbusTestService* service,
    const gchar* name, const gchar* path, const gchar* interface);

void hud_test_utils_json_loader_start (DbusTestService *service,
    const gchar *jsonfile);

void hud_test_utils_json_loader_start_full (DbusTestService *service,
  const gchar *name, const gchar *path, const gchar *jsonfile);

void hud_test_utils_start_dbusmenu_mock_app (DbusTestService **service,
    GDBusConnection **session, const gchar *jsonfile);

void hud_test_utils_start_model_mock_app (DbusTestService **service,
    GDBusConnection **session, const gchar *appname);

gboolean hud_test_utils_timeout_quit_func (gpointer user_data);

void hud_test_utils_process_mainloop (const guint delay);



void hud_test_utils_results_append_func (HudResult *result, gpointer user_data);

gint hud_test_utils_results_compare_func (gconstpointer a, gconstpointer b);

void hud_test_utils_source_assert_result (GPtrArray* results, const guint index,
    const gchar* value);


#endif /* HUDTESTUTILS_H_ */
