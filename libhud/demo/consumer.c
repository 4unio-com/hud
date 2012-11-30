#include "yodawgiheardyoulikeactiongroup.h"

static gboolean quit;

static gboolean
quit_looping (gpointer user_data)
{
  quit = TRUE;

  return G_SOURCE_REMOVE;
}

int
main (int argc, char **argv)
{
  GError *error = NULL;
  GDBusActionGroup *bus;
  GActionGroup *group;

  g_type_init ();

  bus = g_dbus_action_group_get (g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, &error),
                                 argv[1], "/org/gtk/Application/anonymous");
  g_action_group_has_action (G_ACTION_GROUP (bus), "foo");
  g_assert_no_error (error);

  g_timeout_add (100, quit_looping, NULL);
  while (!quit)
    g_main_context_iteration (NULL, TRUE);

  group = yo_dawg_i_heard_you_like_action_group_start (G_ACTION_GROUP (bus), "colourify", NULL);
  g_action_group_change_action_state (group, "red", g_variant_new_int32 (123));
  g_action_group_change_action_state (group, "green", g_variant_new_int32 (234));
  g_action_group_change_action_state (group, "blue", g_variant_new_int32 (50));
  g_action_group_activate_action (group, "apply", NULL);
  g_object_unref (group);

  g_dbus_connection_flush_sync (g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, NULL), NULL, NULL);

  return 0;
}
