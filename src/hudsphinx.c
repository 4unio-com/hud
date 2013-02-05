
#define G_LOG_DOMAIN "hudsphinx"

#include "hudsphinx.h"
#include "hud-query-iface.h"
#include "hudsource.h"

/* Pocket Sphinx */
#include "pocketsphinx.h"
#include <sphinxbase/ad.h>
#include <sphinxbase/cont_ad.h>

#include <gio/gunixoutputstream.h>
#include <glib/gstdio.h>

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

G_DEFINE_TYPE(HudSphinx, hud_sphinx, G_TYPE_OBJECT);

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
  g_debug("finalize:");
  HudSphinx *self = HUD_SPHINX (object);
  g_debug("self=[%p]", self);
  g_debug("self->skel=[%p]", self->skel);
  g_debug("self->alpha=[%p]", self->alphanumeric_regex);

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

  g_debug("new;");
  g_debug("self=[%p]", self);
  g_debug("self->skel=[%p]", self->skel);
  g_debug("self->alpha=[%p]", self->alphanumeric_regex);

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

static gchar *
hud_sphinx_utterance_loop(HudSphinx *self, cmd_ln_t *config, ps_decoder_t *ps)
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
    hud_query_iface_com_canonical_hud_query_emit_voice_query_failed (
        HUD_QUERY_IFACE_COM_CANONICAL_HUD_QUERY (self->skel),
        "Failed to open audio device");
    return NULL;
  }

  /* Initialize continuous listening module */
  if ((cont = cont_ad_init (ad, ad_read)) == NULL )
  {
    hud_query_iface_com_canonical_hud_query_emit_voice_query_failed (
        HUD_QUERY_IFACE_COM_CANONICAL_HUD_QUERY (self->skel),
        "Failed to initialize voice activity detection");
    g_warning("Failed to initialize voice activity detection");
    return NULL;
  }
  if (ad_start_rec (ad) < 0)
  {
    hud_query_iface_com_canonical_hud_query_emit_voice_query_failed (
            HUD_QUERY_IFACE_COM_CANONICAL_HUD_QUERY (self->skel),
            "Failed to start recording");
    g_warning("Failed to start recording");
    return NULL;
  }
  if (cont_ad_calib (cont) < 0)
  {
    hud_query_iface_com_canonical_hud_query_emit_voice_query_failed (
                HUD_QUERY_IFACE_COM_CANONICAL_HUD_QUERY (self->skel),
                "Failed to calibrate voice activity detection");
    g_warning("Failed to calibrate voice activity detection");
    return NULL;
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
    hud_query_iface_com_canonical_hud_query_emit_voice_query_failed (
        HUD_QUERY_IFACE_COM_CANONICAL_HUD_QUERY (self->skel),
        "Failed to read audio");
    return NULL;
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
  while (ad_read (ad, adbuf, 4096) >= 0);;
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
    return g_strdup (hyp);
  }
  else
  {
    return NULL ;
  }

}
/* End code taken from PocketSphinx */

static arg_t sphinx_cmd_ln[] = {
  POCKETSPHINX_OPTIONS,
  {NULL, 0, NULL, NULL}
};

/* Actually recognizing the Audio */
static gchar *
hud_sphinx_recognize_audio(HudSphinx *self, const gchar * lm_filename, const gchar * pron_filename)
{
  cmd_ln_t *config = cmd_ln_init(NULL, sphinx_cmd_ln, TRUE,
                                   "-hmm", "/usr/share/pocketsphinx/model/hmm/en_US/hub4wsj_sc_8k",
                                   "-mdef", "/usr/share/pocketsphinx/model/hmm/en_US/hub4wsj_sc_8k/mdef",
                                   "-lm", lm_filename,
                                   "-dict", pron_filename,
                                   NULL);
  if (config == NULL) {
    g_warning("Sphinx command line arguments failed to initialize");
    return NULL;
  }

  ps_decoder_t *ps = ps_init(config);
  if (ps == NULL) {
    g_warning("Unable to initialize Sphinx decoder");
    return NULL;
  }

  char *hyp = hud_sphinx_utterance_loop(self, config, ps);
  if (hyp == NULL) {
    g_warning("Utterance loop failed");
  } else {
    g_debug("Recognized: %s\n", hyp);
  }

  ps_free(ps);

  return hyp;
}

/* Function to try and get a query from voice */
gchar *
hud_sphinx_voice_query (HudSphinx *self, HudSource *source)
{
  if (source == NULL) {
    /* No active window, that's fine, but we'll just move on */
    return NULL;
  }

  GList * items = hud_source_get_items(source);
  if (items == NULL) {
    /* The active window doesn't have items, that's cool.  We'll move on. */
    return NULL;
  }

  /* Get the pronounciations for the items */
  GHashTable * pronounciations = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify)g_strfreev);
  HudItemPronunciationData pronounciation_data = {pronounciations, self->alphanumeric_regex};
  g_list_foreach(items, (GFunc)hud_item_insert_pronounciation, &pronounciation_data);

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

    gchar *upper = g_utf8_strup(command, g_utf8_strlen(command, -1));
    gchar *filtered = g_regex_replace (self->alphanumeric_regex, upper,
              -1, 0, "", 0, &error);
    if (filtered == NULL) {
      g_error("Regex replace failed: [%s]", error->message);
      g_error_free(error);
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
  if (!g_spawn_command_line_sync(buildlm, NULL, NULL, NULL, NULL))
  {
    g_error("Command [%s] failed", buildlm);
    g_free(buildlm);
    return NULL;
  }
  g_free(buildlm);

  gchar * unzipit = g_strdup_printf("gzip -f -d %s.gz", lm_filename);
  if (!g_spawn_command_line_sync(unzipit, NULL, NULL, NULL, NULL))
  {
    g_error("Command [%s] failed", unzipit);
    g_free(unzipit);
    return NULL;
  }
  g_free(unzipit);

  gchar * retval = hud_sphinx_recognize_audio(self, lm_filename, pron_filename);

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
