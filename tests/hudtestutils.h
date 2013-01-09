#ifndef HUDTESTUTILS_H_
#define HUDTESTUTILS_H_

#include <glib.h>

typedef struct _HudStringList HudStringList;
typedef struct _GDBusConnection GDBusConnection;
typedef struct _DbusTestService DbusTestService;

#define HUD_TEST_UTILS_LOADER_NAME "test.json.loader"
#define HUD_TEST_UTILS_LOADER_PATH "/test/json/loader"

gboolean hud_test_utils_name_timeout (gpointer user_data);

HudStringList* add_item_to_hud_string_list (const gchar *item,
    HudStringList *stringlist);

void hud_test_utils_start_dbusmenu_mock_app (DbusTestService **service,
    GDBusConnection **session, const gchar *jsonfile);

void hud_test_utils_start_model_mock_app (DbusTestService **service,
    GDBusConnection **session, const gchar *appname);

gboolean hud_test_utils_timeout_quit_func (gpointer user_data);

void hud_test_utils_process_mainloop (const guint delay);

#endif /* HUDTESTUTILS_H_ */
