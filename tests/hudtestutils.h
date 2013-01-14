#ifndef HUDTESTUTILS_H_
#define HUDTESTUTILS_H_

#include <glib.h>

typedef struct _HudStringList HudStringList;
typedef struct _GDBusConnection GDBusConnection;
typedef struct _DbusTestService DbusTestService;

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

DBusMockProperties * dbus_mock_new_properties();

DBusMockMethods * dbus_mock_new_methods();

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

gboolean hud_test_utils_name_timeout (gpointer user_data);

void hud_test_utils_start_python_dbusmock (DbusTestService **service,
    GDBusConnection **session, const gchar *name, const gchar *path,
    const gchar *interface);

void hud_test_utils_start_dbusmenu_mock_app (DbusTestService **service,
    GDBusConnection **session, const gchar *jsonfile);

void hud_test_utils_start_model_mock_app (DbusTestService **service,
    GDBusConnection **session, const gchar *appname);

gboolean hud_test_utils_timeout_quit_func (gpointer user_data);

void hud_test_utils_process_mainloop (const guint delay);

#endif /* HUDTESTUTILS_H_ */
