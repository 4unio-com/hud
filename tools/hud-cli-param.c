/*
Small utility to excersise the HUD from the command line

Copyright 2011 Canonical Ltd.

Authors:
    Ted Gould <ted@canonical.com>

This program is free software: you can redistribute it and/or modify it 
under the terms of the GNU General Public License version 3, as published 
by the Free Software Foundation.

This program is distributed in the hope that it will be useful, but 
WITHOUT ANY WARRANTY; without even the implied warranties of 
MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR 
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along 
with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <glib.h>
#include <gio/gunixinputstream.h>
#include <gtk/gtk.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <hud-client.h>


static void print_suggestions(const char * query);
static HudClientQuery * client_query = NULL;

int
main (int argc, char *argv[])
{
#ifndef GLIB_VERSION_2_36
	g_type_init ();
#endif

	const gchar * search = "";

	if (argc == 2) {
		search = argv[1];
	}

	printf("\nsearch token: %s\n", search);
	print_suggestions(search);

	g_clear_object(&client_query);

	return 0;
}

static void
wait_for_sync_notify (GObject * object, GParamSpec * pspec, gpointer user_data)
{
	GMainLoop * loop = (GMainLoop *)user_data;
	g_main_loop_quit(loop);
	return;
}

static gboolean
wait_for_sync (DeeModel * model)
{
	if (dee_shared_model_is_synchronized(DEE_SHARED_MODEL(model))) {
		return TRUE;
	}

	GMainLoop * loop = g_main_loop_new(NULL, FALSE);

	glong sig = g_signal_connect(G_OBJECT(model), "notify::synchronized", G_CALLBACK(wait_for_sync_notify), loop);

	g_main_loop_run(loop);
	g_main_loop_unref(loop);

	g_signal_handler_disconnect(G_OBJECT(model), sig);

	return dee_shared_model_is_synchronized(DEE_SHARED_MODEL(model));
}

static void
print_model (GMenuModel * model)
{
	g_print("Model\n");
	return;
}

static void
model_ready (HudClientParam * param, GMainLoop * loop)
{
	GMenuModel * model = hud_client_param_get_model(param);
	if (model != NULL) {
		print_model(model);
	} else {
		g_warning("Unable to get model even after it was 'ready'");
	}

	g_main_loop_quit(loop);
	return;
}

static void 
print_suggestions (const char *query)
{
	if (client_query == NULL) {
		client_query = hud_client_query_new(query);
	} else {
		hud_client_query_set_query(client_query, query);
	}

	DeeModel * model = hud_client_query_get_results_model(client_query);
	g_return_if_fail(wait_for_sync(model));

	DeeModelIter * iter = NULL;
	int i = 0;
	for (iter = dee_model_get_first_iter(model); !dee_model_is_last(model, iter); iter = dee_model_next(model, iter), i++) {
		if (!dee_model_get_bool(model, iter, 7)) {
			/* Only want parameterized */
			continue;
		}

		const gchar * suggestion = dee_model_get_string(model, iter, 1);
		gchar * clean_line = NULL;
		pango_parse_markup(suggestion, -1, 0, NULL, &clean_line, NULL, NULL);
		printf("\t%s\n", clean_line);
		free(clean_line);

		HudClientParam * param = hud_client_query_execute_param_command(client_query, dee_model_get_value(model, iter, 0), 0);
		g_return_if_fail(param != NULL);

		GMenuModel * model = hud_client_param_get_model(param);
		if (model == NULL) {
			GMainLoop * loop = g_main_loop_new(NULL, FALSE);

			gulong sig = g_signal_connect(param, "model-ready", G_CALLBACK(model_ready), loop);

			g_main_loop_run(loop);
			g_main_loop_unref(loop);

			g_signal_handler_disconnect(param, sig);
		} else {
			print_model(model);
		}

		g_object_unref(param);
	}

	return;
}

