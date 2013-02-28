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

static arg_t sphinx_cmd_ln[] = {
  POCKETSPHINX_OPTIONS,
  {NULL, 0, NULL, NULL}
};

struct _HudSphinx
{
  GObject parent_instance;

  HudQueryIfaceComCanonicalHudQuery *skel;
  GRegex * alphanumeric_regex;
  cmd_ln_t *config;
  ps_decoder_t *ps;
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

  g_clear_pointer(&self->ps, ps_free);

  G_OBJECT_CLASS (hud_sphinx_parent_class)
    ->finalize (object);
}

HudSphinx *
hud_sphinx_new (HudQueryIfaceComCanonicalHudQuery *skel, GError **error)
{
  HudSphinx *self = g_object_new (HUD_TYPE_SPHINX, NULL);
  self->skel = g_object_ref(skel);

  gchar *hmm = "/usr/share/pocketsphinx/model/hmm/en_US/hub4wsj_sc_8k";
  gchar *dict = "/usr/share/pocketsphinx/model/lm/en_US/cmu07a.dic";

  self->config = cmd_ln_init(NULL, sphinx_cmd_ln, TRUE,
                                     "-hmm", hmm,
                                     "-dict", dict,
                                     NULL);

  if (self->config == NULL) {
    g_warning("Sphinx command line arguments failed to initialize");
    *error = g_error_new_literal (hud_sphinx_error_quark (), 0,
        "Sphinx command line arguments failed to initialize");
    return NULL;
  }

  self->ps = ps_init(self->config);
  if (self->ps == NULL) {
    g_warning("Unable to initialize Sphinx decoder");
    *error = g_error_new_literal (hud_sphinx_error_quark (), 0,
        "Unable to initialize Sphinx decoder");
    return NULL;
  }

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
hud_sphinx_utterance_loop(HudSphinx *self, gchar **result, GError **error)
{
  ad_rec_t *ad;
  int16 adbuf[4096];
  int32 k, ts, rem;
  char const *hyp;
  char const *uttid;
  cont_ad_t *cont;

  cmd_ln_t *config = self->config;
  ps_decoder_t *ps = self->ps;

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

  /* Indicate listening for next utterance */
  g_debug("Voice query is listening");
  hud_query_iface_com_canonical_hud_query_emit_voice_query_listening (
      HUD_QUERY_IFACE_COM_CANONICAL_HUD_QUERY (self->skel));

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
  g_debug("Voice query has heard something");
  hud_query_iface_com_canonical_hud_query_emit_voice_query_heard_something(
              HUD_QUERY_IFACE_COM_CANONICAL_HUD_QUERY (self->skel));
  ps_process_raw (ps, adbuf, k, FALSE, FALSE);

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

/* Actually recognizing the Audio */
static gboolean
hud_sphinx_listen (HudSphinx *self, fsg_model_t* fsg,
    gchar **result, GError **error)
{
  // Get the fsg set or create one if none
  fsg_set_t *fsgs = ps_get_fsgset(self->ps);
  if (fsgs == NULL)
    fsgs = ps_update_fsgset(self->ps);

  // Remove the old fsg
  fsg_model_t * old_fsg = fsg_set_get_fsg(fsgs, fsg_model_name(fsg));
  if (old_fsg)
  {
    fsg_set_remove(fsgs, old_fsg);
    fsg_model_free(old_fsg);
  }

  // Add the new fsg
  fsg_set_add(fsgs, fsg_model_name(fsg), fsg);
  fsg_set_select (fsgs, fsg_model_name(fsg));

  ps_update_fsgset (self->ps);

  gboolean success = hud_sphinx_utterance_loop (self, result, error);

  if (success) {
    g_debug("Recognized: %s", *result);
  } else {
    g_warning("Utterance loop failed");
  }

  return success;
}

static void
free_func (gpointer data)
{
  g_ptr_array_free((GPtrArray*) data, TRUE);
}

static gint
hud_sphinx_number_of_states(GPtrArray *command_list)
{
  gint number_of_states = 0;

  guint i;
  for (i = 0; i < command_list->len; ++i)
  {
    GPtrArray *command = g_ptr_array_index(command_list, i);
    number_of_states += command->len;
  }

  // the number of states calculated above doesn't include the start and end
  return number_of_states + 2;
}

static gint
hud_sphinx_write_command (fsg_model_t *fsg, GPtrArray *command,
    gint state_num, gfloat command_probability)
{
  // the first transition goes from the state 0
  // it's probability depends on how many commands there are
  if (command->len > 0)
  {
    const gchar *word = g_ptr_array_index(command, 0);
    gchar *lower = g_utf8_strdown(word, -1);
    gint wid = fsg_model_word_add (fsg, lower);
    fsg_model_trans_add (fsg, 0, ++state_num,
        command_probability, wid);
    g_free(lower);
  }

  // the rest of the transitions are certain (straight path)
  // so have probability 1.0
  guint i;
  for (i = 1; i < command->len; ++i)
  {
    const gchar *word = g_ptr_array_index(command, i);
    gchar *lower = g_utf8_strdown(word, -1);
    gint wid = fsg_model_word_add (fsg, lower);
    fsg_model_trans_add (fsg, state_num, state_num + 1,
        1.0, wid);
    ++state_num;
    g_free(lower);
  }

  // null transition to exit state
  fsg_model_null_trans_add (fsg, state_num, 1, 0);

  return state_num;
}

static gboolean
hud_sphinx_build_grammar (HudSphinx *self, GList *items,
    fsg_model_t **fsg, GError **error)
{
  PronounceDict *dict = pronounce_dict_get_sphinx(error);
  if (dict == NULL)
  {
    return FALSE;
  }

  /* Get the pronounciations for the items */
  GHashTable *pronounciations = g_hash_table_new_full (g_str_hash, g_str_equal,
      g_free, (GDestroyNotify) g_strfreev);
  GPtrArray *command_list = g_ptr_array_new_with_free_func (free_func);
  GHashTable *unique_commands = g_hash_table_new(g_str_hash, g_str_equal);
  HudItemPronunciationData pronounciation_data =
  { pronounciations, self->alphanumeric_regex, command_list, dict, unique_commands };
  g_list_foreach (items, (GFunc) hud_item_insert_pronounciation,
      &pronounciation_data);
  g_hash_table_destroy(unique_commands);

  if (command_list->len == 0)
  {
    *error = g_error_new_literal(hud_sphinx_error_quark(), 0, "Could not build Sphinx grammar. Is sphinx-voxforge installed?");
    g_clear_pointer(&pronounciations, g_hash_table_destroy);
    g_ptr_array_free (command_list, TRUE);
    return FALSE;
  }

  gint number_of_states = hud_sphinx_number_of_states(command_list);
  gfloat command_probability = 1.0f / command_list->len;

  g_debug("Number of states [%d]", number_of_states);

  *fsg = fsg_model_init ("<hud.GRAM>", ps_get_logmath (self->ps),
      cmd_ln_float32_r(self->config, "-lw"), number_of_states);
  (*fsg)->start_state = 0;
  (*fsg)->final_state = 1;

  // starting at state 2 (0 is start and 1 is exit)
  gint state_num = 1;
  guint i;
  for (i = 0; i < command_list->len; ++i)
  {
    GPtrArray *command = g_ptr_array_index(command_list, i);
    // keep a record of the number of states so far
    state_num = hud_sphinx_write_command(*fsg, command, state_num, command_probability);
  }

  glist_t nulls = fsg_model_null_trans_closure (*fsg, NULL );
  glist_free (nulls);

  g_clear_pointer(&pronounciations, g_hash_table_destroy);
  g_ptr_array_free (command_list, TRUE);

  return TRUE;
}

static gboolean
hud_sphinx_voice_query (HudVoice *voice, HudSource *source, gchar **result, GError **error)
{
  g_return_val_if_fail(HUD_IS_SPHINX(voice), FALSE);
  HudSphinx *self = HUD_SPHINX(voice);

  if (source == NULL) {
    /* No active window, that's fine, but we'll just move on */
    *result = NULL;
    *error = g_error_new_literal(hud_sphinx_error_quark(), 0, "Active source is null");
    return FALSE;
  }

  GList *items = hud_source_get_items(source);
  if (items == NULL) {
    /* The active window doesn't have items, that's cool.  We'll move on. */
    return TRUE;
  }

  fsg_model_t *fsg = NULL;
  if (!hud_sphinx_build_grammar(self, items, &fsg, error))
  {
    g_list_free_full(items, g_object_unref);
    return FALSE;
  }

  gboolean success = hud_sphinx_listen (self, fsg, result, error);

  g_list_free_full(items, g_object_unref);

  return success;
}
