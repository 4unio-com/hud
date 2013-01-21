/*
 * Copyright © 2012 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Ryan Lortie <desrt@desrt.ca>
 */

#define G_LOG_DOMAIN "hud-service"

#include "config.h"

#include <glib.h>
#include <gio/gio.h>
#include <gio/gunixoutputstream.h>
#include <glib/gstdio.h>
#include <stdlib.h>
#include <locale.h>
#include <libintl.h>

/* Pocket Sphinx */
#include "pocketsphinx.h"
#include <sphinxbase/ad.h>
#include <sphinxbase/cont_ad.h>

#include "hudappindicatorsource.h"
#include "hudindicatorsource.h"
#include "hudwebappsource.h"
#include "huddebugsource.h"
#include "hudsourcelist.h"
#include "hudsettings.h"
#include "application-list.h"

#include "hud-iface.h"
#include "shared-values.h"
#include "hudquery.h"

/* Prototypes */
static void            bus_method         (GDBusConnection       *connection,
                                           const gchar           *sender,
                                           const gchar           *object_path,
                                           const gchar           *interface_name,
                                           const gchar           *method_name,
                                           GVariant              *parameters,
                                           GDBusMethodInvocation *invocation,
                                           gpointer               user_data);
static GVariant *      bus_get_prop       (GDBusConnection *      connection,
                                           const gchar *          sender,
                                           const gchar *          object_path,
                                           const gchar *          interface_name,
                                           const gchar *          property_name,
                                           GError **              error,
                                           gpointer               user_data);


/* Globals */
static GPtrArray * query_list = NULL;
static GMainLoop *mainloop = NULL;
static GDBusInterfaceVTable vtable = {
	.method_call = bus_method,
	.get_property = bus_get_prop,
	.set_property = NULL
};
static HudApplicationList * application_list = NULL;

/* Get our error domain */
GQuark
error (void)
{
	static GQuark err = 0;
	if (err == 0) {
		err = g_quark_from_static_string("hud");
	}
	return err;
}

/* Start code taken from PocketSphinx */

static char const *
utterance_loop(ad_rec_t * ad, ps_decoder_t * ps)
{
    int16 adbuf[4096];
    int32 k, ts, rem, score;
    char const *hyp;
    char const *uttid;
    cont_ad_t *cont;
    char word[256];

    /* Initialize continuous listening module */
    if ((cont = cont_ad_init(ad, ad_read)) == NULL)
        g_error("cont_ad_init failed\n");
    if (ad_start_rec(ad) < 0)
        g_error("ad_start_rec failed\n");
    if (cont_ad_calib(cont) < 0)
        g_error("cont_ad_calib failed\n");

    if (TRUE) {
        /* Indicate listening for next utterance */
        printf("READY....\n");
        fflush(stdout);
        fflush(stderr);

        /* Await data for next utterance */
        while ((k = cont_ad_read(cont, adbuf, 4096)) == 0)
            g_usleep(200 * 1000);

        if (k < 0)
            g_error("cont_ad_read failed\n");

        /*
         * Non-zero amount of data received; start recognition of new utterance.
         * NULL argument to uttproc_begin_utt => automatic generation of utterance-id.
         */
        if (ps_start_utt(ps, NULL) < 0)
            g_error("ps_start_utt() failed\n");
        ps_process_raw(ps, adbuf, k, FALSE, FALSE);
        printf("Listening...\n");
        fflush(stdout);

        /* Note timestamp for this first block of data */
        ts = cont->read_ts;

        /* Decode utterance until end (marked by a "long" silence, >1sec) */
        for (;;) {
            /* Read non-silence audio data, if any, from continuous listening module */
            if ((k = cont_ad_read(cont, adbuf, 4096)) < 0)
                g_error("cont_ad_read failed\n");
            if (k == 0) {
                /*
                 * No speech data available; check current timestamp with most recent
                 * speech to see if more than 1 sec elapsed.  If so, end of utterance.
                 */
                if ((cont->read_ts - ts) > DEFAULT_SAMPLES_PER_SEC)
                    break;
            }
            else {
                /* New speech data received; note current timestamp */
                ts = cont->read_ts;
            }

            /*
             * Decode whatever data was read above.
             */
            rem = ps_process_raw(ps, adbuf, k, FALSE, FALSE);

            /* If no work to be done, sleep a bit */
            if ((rem == 0) && (k == 0))
                g_usleep(20 * 1000);
        }

        /*
         * Utterance ended; flush any accumulated, unprocessed A/D data and stop
         * listening until current utterance completely decoded
         */
        ad_stop_rec(ad);
        while (ad_read(ad, adbuf, 4096) >= 0);
        cont_ad_reset(cont);

        printf("Stopped listening, please wait...\n");
        fflush(stdout);
        /* Finish decoding, obtain and print result */
        ps_end_utt(ps);
        hyp = ps_get_hyp(ps, &score, &uttid);
        printf("%s: %s (%d)\n", uttid, hyp, score);
        fflush(stdout);

        /* Exit if the first word spoken was GOODBYE */
        if (hyp) {
            sscanf(hyp, "%s", word);
            if (strcmp(word, "goodbye") == 0)
                return NULL;
        }

        /* Resume A/D recording for next utterance */
        if (ad_start_rec(ad) < 0)
            g_error("ad_start_rec failed\n");
    }

    cont_ad_close(cont);
	return hyp;
}
/* End code taken from PocketSphinx */

static arg_t sphinx_cmd_ln[] = {
	POCKETSPHINX_OPTIONS,
	{NULL, 0, NULL, NULL}
};

/* Actually recognizing the Audio */
static gchar *
recognize_audio(const gchar * lm_filename, const gchar * pron_filename)
{
	cmd_ln_t * spx_cmd = cmd_ln_init(NULL, sphinx_cmd_ln, TRUE,
	                                 "-hmm", "/usr/share/pocketsphinx/model/hmm/wsj1",
	                                 "-mdef", "/usr/share/pocketsphinx/model/hmm/wsj1/mdef",
	                                 "-lm", lm_filename,
	                                 "-dict", pron_filename,
	                                 NULL);
	ps_decoder_t * decoder = ps_init(spx_cmd);

	ad_rec_t * ad = ad_open_dev(cmd_ln_str_r(spx_cmd, "-adcdev"), (int)cmd_ln_float32_r(spx_cmd, "-samprate"));
	if (ad == NULL) {
		g_warning("Unable to get audio device");
		ps_free(decoder);
		return NULL;
	}

	char const * hyp;

	hyp = utterance_loop(ad, decoder);
	if (hyp == NULL) {
		return NULL;
	}

	g_debug("Recognized: %s\n", hyp);
	gchar * retval = g_strdup(hyp);

	ps_free(decoder);
	ad_close(ad);

	return retval;
}

/* Function to try and get a query from voice */
static gchar *
do_voice (HudSource * source_kinda)
{
	HudCollector * collector = hud_source_list_active_collector(HUD_SOURCE_LIST(source_kinda));
	if (collector == NULL) {
		/* No active window, that's fine, but we'll just move on */
		return NULL;
	}

	GList * items = hud_collector_get_items(collector);
	if (items == NULL) {
		/* The active window doesn't have items, that's cool.  We'll move on. */
		return NULL;
	}

	/* Get the pronounciations for the items */
	GHashTable * pronounciations = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify)g_strfreev);
	g_list_foreach(items, (GFunc)hud_item_insert_pronounciation, pronounciations);

	/* Get our cache together */
	gchar * string_filename = NULL;
	gchar * pron_filename = NULL;
	gchar * lm_filename = NULL;

	gint string_file = 0;
	gint pron_file = 0;
	gint lm_file = 0;

	GError * error = NULL;

	if ((string_file = g_file_open_tmp("hud-strings-XXXXXX.txt", &string_filename, &error)) == 0 ||
			(pron_file = g_file_open_tmp("hud-pronounciations-XXXXXX.txt", &pron_filename, &error)) == 0 ||
			(lm_file = g_file_open_tmp("hud-lang-XXXXXX.lm", &lm_filename, &error)) == 0) {
		g_warning("Unable to open temporary filee: %s", error->message);

		if (string_file != 0) {
			close(string_file);
			g_unlink(string_filename);
			g_free(string_filename);
		}

		if (pron_file != 0) {
			close(pron_file);
			g_unlink(pron_filename);
			g_free(pron_filename);
		}

		if (lm_file != 0) {
			close(lm_file);
			g_unlink(lm_filename);
			g_free(lm_filename);
		}

		g_error_free(error);
		g_hash_table_unref(pronounciations);
		return NULL;
	}

	/* Now we have files -- now streams */
	GOutputStream * pron_output = g_unix_output_stream_new(pron_file, FALSE);

	/* Go through all of the pronounciations */
	GHashTableIter iter;
	g_hash_table_iter_init(&iter, pronounciations);
	gpointer key, value;
	while (g_hash_table_iter_next(&iter, &key, &value)) {
		gchar ** prons = (gchar **)value;
		gint i;

		for (i = 0; prons[i] != NULL; i++) {
			g_output_stream_write(pron_output, key, g_utf8_strlen(key, -1), NULL, NULL);
			if (i != 0) {
				gchar * number = g_strdup_printf("(%d)", i + 1);
				g_output_stream_write(pron_output, number, g_utf8_strlen(number, -1), NULL, NULL);
				g_free(number);
			}
			g_output_stream_write(pron_output, "\t", g_utf8_strlen("\t", -1), NULL, NULL);
			g_output_stream_write(pron_output, prons[i], g_utf8_strlen(prons[i], -1), NULL, NULL);
			g_output_stream_write(pron_output, "\n", g_utf8_strlen("\n", -1), NULL, NULL);
		}
	}

	g_hash_table_unref(pronounciations);
	g_clear_object(&pron_output);

	/* Get the commands from the items */
	GOutputStream * string_output = g_unix_output_stream_new(string_file, FALSE);
	GList * litem;

	for (litem = items; litem != NULL; litem = g_list_next(litem)) {
		HudItem * item = HUD_ITEM(litem->data);

		const gchar * command = hud_item_get_command(item);
		if (command == NULL) {
			continue;
		}

		g_output_stream_write(string_output, "<s> ", g_utf8_strlen("<s> ", -1), NULL, NULL);
		g_output_stream_write(string_output, command, g_utf8_strlen(command, -1), NULL, NULL);
		g_output_stream_write(string_output, " </s>\n", g_utf8_strlen(" </s>\n", -1), NULL, NULL);
	}

	g_clear_object(&string_output);

	if (string_file != 0) {
		close(string_file);
	}

	if (pron_file != 0) {
		close(pron_file);
	}

	if (lm_file != 0) {
		close(lm_file);
	}

	g_debug("String: %s", string_filename);
	g_debug("Pronounciations: %s", pron_filename);
	g_debug("Lang Model: %s", lm_filename);

	/* Okay, now some shell stuff */
	g_setenv("IRSTLM", "/usr", TRUE);

	gchar * buildlm = g_strdup_printf("/usr/bin/build-lm.sh -i %s -o %s.gz", string_filename, lm_filename);
	g_spawn_command_line_sync(buildlm, NULL, NULL, NULL, NULL);
	g_free(buildlm);

	gchar * unzipit = g_strdup_printf("gzip -f -d %s.gz", lm_filename);
	g_spawn_command_line_sync(unzipit, NULL, NULL, NULL, NULL);
	g_free(unzipit);

	gchar * retval = recognize_audio(lm_filename, pron_filename);

	if (string_file != 0) {
		g_unlink(string_filename);
	}

	if (pron_file != 0) {
		g_unlink(pron_filename);
	}

	if (lm_file != 0) {
		g_unlink(lm_filename);
	}

	g_free(string_filename);
	g_free(pron_filename);
	g_free(lm_filename);

	return retval;
}

/* Describe the important values in the query */
GVariant *
describe_query (HudQuery *query)
{
  GVariantBuilder builder;

  g_variant_builder_init (&builder, G_VARIANT_TYPE_TUPLE);

  g_variant_builder_add_value(&builder, g_variant_new_object_path(hud_query_get_path(query)));
  g_variant_builder_add_value(&builder, g_variant_new_string(hud_query_get_results_name(query)));
  g_variant_builder_add_value(&builder, g_variant_new_string(hud_query_get_appstack_name(query)));
  g_variant_builder_add_value(&builder, g_variant_new_int32(0)); /* TODO: Get model rev */

  return g_variant_builder_end (&builder);
}

static void
query_destroyed (gpointer data, GObject * old_object)
{
	GPtrArray * list = (GPtrArray *)data;
	g_ptr_array_remove(list, old_object);
	return;
}

static void
bus_method (GDBusConnection       *connection,
            const gchar           *sender,
            const gchar           *object_path,
            const gchar           *interface_name,
            const gchar           *method_name,
            GVariant              *parameters,
            GDBusMethodInvocation *invocation,
            gpointer               user_data)
{
	HudSource *source = user_data;

	if (g_str_equal (method_name, "StartQuery")) {
		GVariant * vsearch;
		const gchar *search_string;
		HudQuery *query;

		vsearch = g_variant_get_child_value (parameters, 0);
		search_string = g_variant_get_string(vsearch, NULL);
		g_debug ("'StartQuery' from %s: '%s'", sender, search_string);

		query = hud_query_new (source, search_string, 10, connection);
		g_dbus_method_invocation_return_value (invocation, describe_query (query));

		g_ptr_array_add(query_list, query);
		g_object_weak_ref(G_OBJECT(query), query_destroyed, query_list);

		g_variant_unref(vsearch);
	} else if (g_str_equal (method_name, "RegisterApplication")) {
		GVariant * vid = g_variant_get_child_value (parameters, 0);

		HudApplicationSource * appsource = hud_application_list_get_source(application_list, g_variant_get_string(vid, NULL));

		const gchar * path = NULL;
		if (appsource != NULL) {
			path = hud_application_source_get_path(appsource);
		}

		GVariant * vpath = NULL;
		if (path != NULL) {
			vpath = g_variant_new_object_path(path);
		}

		if (vpath != NULL) {
			g_dbus_method_invocation_return_value(invocation, g_variant_new_tuple(&vpath, 1));
		} else {
			g_dbus_method_invocation_return_error_literal(invocation, error(), 1, "Unable to get path for the created application");
		}
		g_variant_unref(vid);
	} else if (g_str_equal (method_name, "VoiceSearch")) {
    gchar * querystr = do_voice(source);

    GVariant * str = NULL;
    if (querystr == NULL) {
      str = g_variant_new_string("");
    } else {
      str = g_variant_new_string(querystr);
    }
    g_dbus_method_invocation_return_value (invocation, g_variant_new_tuple(&str, 1));
	} else {
		g_warn_if_reached();
	}

	return;
}

/* Gets properties for DBus */
static GVariant *
bus_get_prop (GDBusConnection * connection, const gchar * sender, const gchar * object_path, const gchar * interface_name, const gchar * property_name, GError ** error, gpointer user_data)
{
	// HudSource *source = user_data;

	if (g_str_equal(property_name, "OpenQueries")) {
		if (query_list->len == 0) {
			return g_variant_new_array(G_VARIANT_TYPE_OBJECT_PATH, NULL, 0);
		}

		int i;
		GVariantBuilder builder;
		g_variant_builder_init(&builder, G_VARIANT_TYPE_ARRAY);

		for (i = 0; i < query_list->len; i++) {
			HudQuery * query = g_ptr_array_index(query_list, i);
			g_variant_builder_add_value(&builder, g_variant_new_object_path(hud_query_get_path(query)));
		}

		return g_variant_builder_end(&builder);
	} else if (g_str_equal(property_name, "Applications")) {
		GList * apps = hud_application_list_get_apps(application_list);
		if (apps == NULL) {
			return g_variant_new_array(G_VARIANT_TYPE_OBJECT_PATH, NULL, 0);
		}

		GVariantBuilder builder;
		g_variant_builder_init(&builder, G_VARIANT_TYPE_ARRAY);
		while (apps != NULL) {
			HudApplicationSource * source = HUD_APPLICATION_SOURCE(apps->data);
			g_variant_builder_open(&builder, G_VARIANT_TYPE_TUPLE);
			g_variant_builder_add_value(&builder, g_variant_new_string(hud_application_source_get_id(source)));
			g_variant_builder_add_value(&builder, g_variant_new_object_path(hud_application_source_get_path(source)));
			g_variant_builder_close(&builder);
			apps = g_list_next(apps);
		}

		return g_variant_builder_end(&builder);
	} else {
		g_warn_if_reached();
	}

	return NULL;
}

static void
bus_acquired_cb (GDBusConnection *connection,
                 const gchar     *name,
                 gpointer         user_data)
{
  HudSourceList *source_list = user_data;
  GError *error = NULL;

  g_debug ("Bus acquired (guid %s)", g_dbus_connection_get_guid (connection));

  {
    HudIndicatorSource *source;

    source = hud_indicator_source_new (connection);
    hud_source_list_add (source_list, HUD_SOURCE (source));
    g_object_unref (source);
  }

  {
    HudAppIndicatorSource *source;

    source = hud_app_indicator_source_new (connection);
    hud_source_list_add (source_list, HUD_SOURCE (source));
    g_object_unref (source);
  }

  {
    HudWebappSource *source;

    source = hud_webapp_source_new ();
    hud_source_list_add (source_list, HUD_SOURCE (source));

    g_object_unref (G_OBJECT (source));
  }

  if (!g_dbus_connection_register_object (connection, DBUS_PATH, hud_iface_com_canonical_hud_interface_info (), &vtable, source_list, NULL, &error))
    {
      g_warning ("Unable to register path '"DBUS_PATH"': %s", error->message);
      g_main_loop_quit (mainloop);
      g_error_free (error);
    }
}

static void
name_acquired_cb (GDBusConnection *connection,
                  const gchar     *name,
                  gpointer         user_data)
{
  g_debug ("Acquired bus name '%s'", name);
}

static void
name_lost_cb (GDBusConnection *connection,
              const gchar     *name,
              gpointer         user_data)
{
  g_warning ("Unable to get name '%s'", name);
  g_main_loop_quit (mainloop);
}

static void
sigterm_graceful_exit (int signal)
{
  g_warning("SIGTERM recieved");
  g_main_loop_quit(mainloop);
  return;
}

int
main (int argc, char **argv)
{
  HudSourceList *source_list;

#ifndef GLIB_VERSION_2_36
  g_type_init ();
#endif

  setlocale (LC_ALL, "");
  bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
  textdomain (GETTEXT_PACKAGE);

  hud_settings_init ();

  query_list = g_ptr_array_new();
  source_list = hud_source_list_new ();

  application_list = hud_application_list_new();
  hud_source_list_add(source_list, HUD_SOURCE(application_list));

  if (getenv ("HUD_DEBUG_SOURCE"))
    {
      HudDebugSource *source;

      source = hud_debug_source_new ();
      hud_source_list_add (source_list, HUD_SOURCE (source));
      g_object_unref (source);
    }

  g_bus_own_name (G_BUS_TYPE_SESSION, DBUS_NAME, G_BUS_NAME_OWNER_FLAGS_NONE,
                  bus_acquired_cb, name_acquired_cb, name_lost_cb, source_list, NULL);

  mainloop = g_main_loop_new (NULL, FALSE);

  signal(SIGTERM, sigterm_graceful_exit);
  
  g_main_loop_run (mainloop);
  g_main_loop_unref (mainloop);

  g_object_unref (application_list);
  g_object_unref (source_list);
  g_ptr_array_free(query_list, TRUE);

  return 0;
}
