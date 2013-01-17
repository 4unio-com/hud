#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#include "hud.h"

int
main (int argc, char * argv[])
{
	gtk_init(&argc, &argv);

	GtkWidget * window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "Simple Test");
	gtk_widget_show(window);
	gtk_widget_realize(window);

	GdkWindow * gdkwindow = gtk_widget_get_window(window);

	HudManager * manager = hud_manager_new("simple-test");

	GSimpleActionGroup * actions = g_simple_action_group_new();
	g_simple_action_group_insert(actions, G_ACTION(g_simple_action_new("simple-action", NULL)));

	g_dbus_connection_export_action_group(g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, NULL),
	                                      "/actions",
	                                      G_ACTION_GROUP(actions),
	                                      NULL);

	HudActionDescription * desc = hud_action_description_new("hud.simple-action", NULL);
	hud_action_description_set_attribute_value(desc, G_MENU_ATTRIBUTE_LABEL, g_variant_new_string("Simple Action"));

	HudActionPublisher * publisher = hud_action_publisher_new_for_id(g_variant_new_int32(GDK_WINDOW_XID(gdkwindow)));
	hud_action_publisher_add_action_group(publisher, "hud", "/actions");
	hud_action_publisher_add_description(publisher, desc);

	hud_manager_add_actions(manager, publisher);

	GMainLoop * mainloop = g_main_loop_new(NULL, FALSE);

	g_main_loop_run(mainloop);

	g_main_loop_unref(mainloop);
	g_object_unref(publisher);
	g_object_unref(desc);
	g_object_unref(actions);
	g_object_unref(manager);

	return 0;
}
