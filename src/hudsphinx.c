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
 */

#define G_LOG_DOMAIN "hudsphinx"

#include "hudvoice.h"
#include "hudsphinx.h"
#include "hud-query-iface.h"
#include "hudsource.h"
#include "pronounce-dict.h"

/* Pocket Sphinx */
#include "pocketsphinx.h"
#include <sphinxbase/ad.h>
#include <sphinxbase/cont_ad.h>

#include <gio/gunixoutputstream.h>
#include <glib/gstdio.h>

static GQuark
hud_sphinx_error_quark(void)
{
  static GQuark quark = 0;
  if (quark == 0)
    quark = g_quark_from_static_string ("hud-sphinx-error-quark");
  return quark;
}

static GRegex *
hud_sphinx_alphanumeric_regex_new (void)
{
  GRegex *alphanumeric_regex = NULL;

  GError *error = NULL;
  alphanumeric_regex = g_regex_new("…|\\.\\.\\.", 0, 0, &error);
  if (alphanumeric_regex == NULL) {
    g_error("Compiling regex failed: [%s]", error->message);
    g_error_free(error);
  }

  return alphanumeric_regex;
}

struct _HudSphinx
{
  GObject parent_instance;

  HudQueryIfaceComCanonicalHudQuery *skel;
  GRegex * alphanumeric_regex;
};

typedef GObjectClass HudSphinxClass;

static void hud_sphinx_finalize (GObject *object);

static gboolean hud_sphinx_voice_query (HudVoice *self, HudSource *source,
    gchar **result, GError **error);

static void hud_sphinx_iface_init (HudVoiceInterface *iface);

G_DEFINE_TYPE_WITH_CODE (HudSphinx, hud_sphinx, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (HUD_TYPE_VOICE, hud_sphinx_iface_init))

static void hud_sphinx_iface_init (HudVoiceInterface *iface)
{
  iface->query = hud_sphinx_voice_query;
}

static void
hud_sphinx_class_init (GObjectClass *klass)
{
  klass->finalize = hud_sphinx_finalize;
}

static void
hud_sphinx_init (HudSphinx *self)
{
  self->alphanumeric_regex = hud_sphinx_alphanumeric_regex_new();
}

static void
hud_sphinx_finalize (GObject *object)
{
  HudSphinx *self = HUD_SPHINX (object);

  g_clear_object(&self->skel);
  g_clear_pointer(&self->alphanumeric_regex, g_regex_unref);

  G_OBJECT_CLASS (hud_sphinx_parent_class)
    ->finalize (object);
}

HudSphinx *
hud_sphinx_new (HudQueryIfaceComCanonicalHudQuery * skel)
{
  HudSphinx *self = g_object_new (HUD_TYPE_SPHINX, NULL);
  self->skel = g_object_ref(skel);

  return self;
}


/* Start code taken from PocketSphinx */

/* Sleep for specified msec */
static void
sleep_msec(int32 ms)
{
#if (defined(WIN32) && !defined(GNUWINCE)) || defined(_WIN32_WCE)
    Sleep(ms);
#else
    /* ------------------- Unix ------------------ */
    struct timeval tmo;

    tmo.tv_sec = 0;
    tmo.tv_usec = ms * 1000;

    select(0, NULL, NULL, NULL, &tmo);
#endif
}

static gboolean
hud_sphinx_utterance_loop(HudSphinx *self, cmd_ln_t *config, ps_decoder_t *ps, gchar **result, GError **error)
{
  ad_rec_t *ad;
  int16 adbuf[4096];
  int32 k, ts, rem;
  char const *hyp;
  char const *uttid;
  cont_ad_t *cont;

  if ((ad = ad_open_dev (cmd_ln_str_r (config, "-adcdev"),
      (int) cmd_ln_float32_r(config, "-samprate"))) == NULL )
  {
    g_warning("Failed to open audio device");
    *result = NULL;
    *error = g_error_new_literal (hud_sphinx_error_quark (), 0,
        "Failed to open audio device");
    return FALSE;
  }

  /* Initialize continuous listening module */
  if ((cont = cont_ad_init (ad, ad_read)) == NULL )
  {
    g_warning("Failed to initialize voice activity detection");
    *result = NULL;
    *error = g_error_new_literal (hud_sphinx_error_quark (), 0,
        "Failed to initialize voice activity detection");
    return FALSE;
  }
  if (ad_start_rec (ad) < 0)
  {
    g_warning("Failed to start recording");
    *result = NULL;
    *error = g_error_new_literal (hud_sphinx_error_quark (), 0,
        "Failed to start recording");
    return FALSE;
  }
  if (cont_ad_calib (cont) < 0)
  {
    g_warning("Failed to calibrate voice activity detection");
    *result = NULL;
    *error = g_error_new_literal (hud_sphinx_error_quark (), 0,
        "Failed to calibrate voice activity detection");
    return FALSE;
  }

  /* Indicate listening for next utterance */
  g_debug("Voice query is listening");
  hud_query_iface_com_canonical_hud_query_emit_voice_query_listening (
      HUD_QUERY_IFACE_COM_CANONICAL_HUD_QUERY (self->skel));
  fflush (stdout);
  fflush (stderr);

  /* Wait data for next utterance */
  while ((k = cont_ad_read (cont, adbuf, 4096)) == 0)
    sleep_msec (100);

  if (k < 0)
  {
    g_warning("Failed to read audio");
    *result = NULL;
    *error = g_error_new_literal (hud_sphinx_error_quark (), 0,
        "Failed to read audio");
    return FALSE;
  }

  /*
   * Non-zero amount of data received; start recognition of new utterance.
   * NULL argument to uttproc_begin_utt => automatic generation of utterance-id.
   */
  if (ps_start_utt (ps, NULL ) < 0)
    g_error("Failed to start utterance");
  ps_process_raw (ps, adbuf, k, FALSE, FALSE);
  g_debug("Voice query has heard something");
  hud_query_iface_com_canonical_hud_query_emit_voice_query_heard_something(
              HUD_QUERY_IFACE_COM_CANONICAL_HUD_QUERY (self->skel));
  fflush (stdout);

  /* Note timestamp for this first block of data */
  ts = cont->read_ts;

  /* Decode utterance until end (marked by a "long" silence, >1sec) */
  for (;;)
  {
    /* Read non-silence audio data, if any, from continuous listening module */
    if ((k = cont_ad_read (cont, adbuf, 4096)) < 0)
      g_error("Failed to read audio");
    if (k == 0)
    {
      /*
       * No speech data available; check current timestamp with most recent
       * speech to see if more than 1 sec elapsed.  If so, end of utterance.
       */
      if ((cont->read_ts - ts) > DEFAULT_SAMPLES_PER_SEC)
        break;
    }
    else
    {
      /* New speech data received; note current timestamp */
      ts = cont->read_ts;
    }

    /*
     * Decode whatever data was read above.
     */
    rem = ps_process_raw (ps, adbuf, k, FALSE, FALSE);

    /* If no work to be done, sleep a bit */
    if ((rem == 0) && (k == 0))
      sleep_msec (20);
  }

  /*
   * Utterance ended; flush any accumulated, unprocessed A/D data and stop
   * listening until current utterance completely decoded
   */
  ad_stop_rec (ad);
  while (ad_read (ad, adbuf, 4096) >= 0);
  cont_ad_reset (cont);

  g_debug("Voice query has stopped listening, processing...");
  fflush (stdout);
  /* Finish decoding, obtain and print result */
  ps_end_utt (ps);
  hyp = ps_get_hyp (ps, NULL, &uttid);
  fflush (stdout);

  cont_ad_close (cont);
  ad_close (ad);

  if (hyp)
  {
    *result = g_strdup (hyp);
  }
  else
  {
    *result = NULL;
  }

  return TRUE;

}
/* End code taken from PocketSphinx */

static arg_t sphinx_cmd_ln[] = {
  POCKETSPHINX_OPTIONS,
  {NULL, 0, NULL, NULL}
};

/* Actually recognizing the Audio */
static gboolean
hud_sphinx_recognize_audio(HudSphinx *self, const gchar * lm_filename, const gchar * pron_filename, gchar **result, GError **error)
{
  cmd_ln_t *config = cmd_ln_init(NULL, sphinx_cmd_ln, TRUE,
                                   "-hmm", "/usr/share/pocketsphinx/model/hmm/en_US/hub4wsj_sc_8k",
                                   "-mdef", "/usr/share/pocketsphinx/model/hmm/en_US/hub4wsj_sc_8k/mdef",
                                   "-lm", lm_filename,
                                   "-dict", pron_filename,
                                   NULL);
  if (config == NULL) {
    g_warning("Sphinx command line arguments failed to initialize");
    *result = NULL;
    *error = g_error_new_literal (hud_sphinx_error_quark (), 0,
        "Sphinx command line arguments failed to initialize");
    return FALSE;
  }

  ps_decoder_t *ps = ps_init(config);
  if (ps == NULL) {
    g_warning("Unable to initialize Sphinx decoder");
    *result = NULL;
    *error = g_error_new_literal (hud_sphinx_error_quark (), 0,
        "Unable to initialize Sphinx decoder");
    return FALSE;
  }

  gboolean success = hud_sphinx_utterance_loop (self, config, ps, result,
      error);

  if (success) {
    g_debug("Recognized: %s", *result);
  } else {
    g_warning("Utterance loop failed");
  }

  ps_free(ps);
  return success;
}

static void
free_func (gpointer data)
{
  g_ptr_array_free((GPtrArray*) data, TRUE);
}

/* Function to try and get a query from voice */
static gboolean
hud_sphinx_voice_query (HudVoice *voice, HudSource *source,
    gchar **result, GError **error)
{
  g_return_val_if_fail(HUD_IS_SPHINX(voice), FALSE);
  HudSphinx *self = HUD_SPHINX(voice);

  if (source == NULL) {
    /* No active window, that's fine, but we'll just move on */
    *result = NULL;
    *error = g_error_new_literal(hud_sphinx_error_quark(), 0, "Active source is null");
    return FALSE;
  }

  GList * items = hud_source_get_items(source);
  if (items == NULL) {
    /* The active window doesn't have items, that's cool.  We'll move on. */
    *result = NULL;
    return TRUE;
  }

  PronounceDict *dict = pronounce_dict_get_sphinx(error);
  if (dict == NULL)
  {
    g_list_free_full(items, g_object_unref);
    *result = NULL;
    return FALSE;
  }

  /* Get the pronounciations for the items */
  GHashTable * pronounciations = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify)g_strfreev);
  GPtrArray *command_list = g_ptr_array_new_with_free_func(free_func);
  GHashTable *unique_commands = g_hash_table_new(g_str_hash, g_str_equal);
  HudItemPronunciationData pronounciation_data = {pronounciations, self->alphanumeric_regex, command_list, dict, unique_commands};
  g_list_foreach(items, (GFunc)hud_item_insert_pronounciation, &pronounciation_data);
  g_hash_table_destroy(unique_commands);
  g_ptr_array_free(command_list, TRUE);

  /* Get our cache together */
  gchar * string_filename = NULL;
  gchar * pron_filename = NULL;
  gchar * lm_filename = NULL;

  gint string_file = 0;
  gint pron_file = 0;
  gint lm_file = 0;

  *error = NULL;
  if ((string_file = g_file_open_tmp("hud-strings-XXXXXX.txt", &string_filename, error)) == 0 ||
      (pron_file = g_file_open_tmp("hud-pronounciations-XXXXXX.txt", &pron_filename, error)) == 0 ||
      (lm_file = g_file_open_tmp("hud-lang-XXXXXX.lm", &lm_filename, error)) == 0) {
    g_warning("Unable to open temporary filee: %s", (*error)->message);

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

    g_hash_table_unref(pronounciations);
    *result = NULL;
    return FALSE;
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

    gchar *upper = g_utf8_strup(command, g_utf8_strlen(command, -1));
    *error = NULL;
    gchar *filtered = g_regex_replace (self->alphanumeric_regex, upper,
              -1, 0, "", 0, error);
    if (filtered == NULL) {
      g_error("Regex replace failed: [%s]", (*error)->message);
      g_free(filtered);
      g_free(upper);
      *result = NULL;
      g_list_free_full(items, g_object_unref);
      return FALSE;
    }

    g_output_stream_write(string_output, "<s> ", g_utf8_strlen("<s> ", -1), NULL, NULL);
    g_output_stream_write(string_output, filtered, g_utf8_strlen(filtered, -1), NULL, NULL);
    g_output_stream_write(string_output, " </s>\n", g_utf8_strlen(" </s>\n", -1), NULL, NULL);

    g_free(filtered);
    g_free(upper);
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

  g_list_free_full(items, g_object_unref);

  g_debug("String: %s", string_filename);
  g_debug("Pronounciations: %s", pron_filename);
  g_debug("Lang Model: %s", lm_filename);

  /* Okay, now some shell stuff */
  g_setenv("IRSTLM", "/usr", TRUE);

  gchar * buildlm = g_strdup_printf("/usr/bin/build-lm.sh -i %s -o %s.gz", string_filename, lm_filename);
  if (!g_spawn_command_line_sync(buildlm, NULL, NULL, NULL, error))
  {
    g_warning("Command [%s] failed", buildlm);
    g_free(buildlm);
    *result = NULL;
    return FALSE;
  }
  g_free(buildlm);

  gchar * unzipit = g_strdup_printf("gzip -f -d %s.gz", lm_filename);
  if (!g_spawn_command_line_sync(unzipit, NULL, NULL, NULL, error))
  {
    g_error("Command [%s] failed", unzipit);
    g_free(unzipit);
    *result = NULL;
    return FALSE;
  }
  g_free(unzipit);

  gboolean success = hud_sphinx_recognize_audio(self, lm_filename, pron_filename, result, error);

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

  return success;
}
