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

#define G_LOG_DOMAIN "hudjulius"

#include "hudjulius.h"
#include "hudsource.h"
#include "pronounce-dict.h"
#include "hud-query-iface.h"

#include <julius/juliuslib.h>
#include <glib/gstdio.h>
#include <gio/gio.h>
#include <signal.h>

struct _HudJulius
{
  GObject parent_instance;

  HudQueryIfaceComCanonicalHudQuery * skel;

  GRegex * alphanumeric_regex;

  gchar **query;

  gboolean listen_emitted;

  gboolean heard_something_emitted;

  GMainLoop *mainloop;

  guint timeout_source;

  guint watch_source;

  GError **error;
};

static GQuark
hud_julius_error_quark(void)
{
  static GQuark quark = 0;
  if (quark == 0)
    quark = g_quark_from_static_string ("hud-julius-error-quark");
  return quark;
}

static GRegex *
hud_julius_alphanumeric_regex_new (void)
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

static void rm_rf(const gchar *path);

typedef GObjectClass HudJuliusClass;

static void hud_julius_finalize (GObject *object);

G_DEFINE_TYPE(HudJulius, hud_julius, G_TYPE_OBJECT);

static void
hud_julius_class_init (HudJuliusClass *klass)
{
  klass->finalize = hud_julius_finalize;
}

static void
hud_julius_init (HudJulius *self)
{
  self->alphanumeric_regex = hud_julius_alphanumeric_regex_new();
}

static void
hud_julius_finalize (GObject *object)
{
  HudJulius *self = HUD_JULIUS (object);

  g_clear_object(&self->skel);
  g_clear_pointer(&self->alphanumeric_regex, g_regex_unref);

  G_OBJECT_CLASS (hud_julius_parent_class)
    ->finalize (object);
}


static gboolean
timeout_quit_func (gpointer user_data)
{
  g_assert(HUD_IS_JULIUS(user_data));

  HudJulius *self = HUD_JULIUS(user_data);
  g_debug("Query timeout");
  *self->query = g_strdup("");
  g_source_remove(self->watch_source);
  g_main_loop_quit(self->mainloop);
  return FALSE;
}

static gboolean
watch_function (GIOChannel *channel, GIOCondition condition,
    gpointer user_data)
{
  g_assert(HUD_IS_JULIUS(user_data));

  HudJulius *self = HUD_JULIUS(user_data);

  if ((condition & G_IO_IN) != 0)
  {
    GString *line = g_string_sized_new (100);
    GIOStatus status;

    do
    {
      do
      {
        *self->error = NULL;
        status = g_io_channel_read_line_string (channel, line, NULL,
            self->error);
      }
      while (status == G_IO_STATUS_AGAIN);

      if (status != G_IO_STATUS_NORMAL)
      {
        if (*self->error)
        {
          g_warning("IO ERROR(): %s", (*self->error)->message);
          return FALSE;
        }
      }

      if (g_str_has_prefix (line->str, "<voice-query-listening/>"))
      {
        if (!self->listen_emitted)
        {
          self->listen_emitted = TRUE;
          hud_query_iface_com_canonical_hud_query_emit_voice_query_listening (
              HUD_QUERY_IFACE_COM_CANONICAL_HUD_QUERY (self->skel) );
          g_debug("<<< please speak >>>");
        }
      }
      else if (g_str_has_prefix (line->str, "<voice-query-heard-something/>"))
      {
        if (!self->heard_something_emitted)
        {
          self->heard_something_emitted = TRUE;
          hud_query_iface_com_canonical_hud_query_emit_voice_query_heard_something (
              HUD_QUERY_IFACE_COM_CANONICAL_HUD_QUERY (self->skel) );
          g_debug("<<< speech input >>>");
        }
      }
      else if (g_str_has_prefix (line->str, "<voice-query-finished>"))
      {
        gchar *tmp = g_string_free (line, FALSE);
        GRegex *regex = g_regex_new("<voice-query-finished>|</voice-query-finished>", 0, 0, NULL);
        *self->query = g_regex_replace_literal(regex, g_strstrip(tmp), -1, 0, "", 0, NULL);
        g_regex_unref(regex);
        g_free(tmp);
        g_source_remove(self->timeout_source);
        g_main_loop_quit(self->mainloop);
        return FALSE;
      }
    }
    while (g_io_channel_get_buffer_condition (channel) == G_IO_IN);
    g_string_free (line, TRUE);

  }

  if ((condition & G_IO_HUP) != 0)
  {
    g_io_channel_shutdown (channel, TRUE, NULL );
    *self->query = NULL;
    *self->error = g_error_new_literal(hud_julius_error_quark(), 0, "HUD Julius listening daemon failed");
    g_source_remove(self->timeout_source);
    g_main_loop_quit(self->mainloop);
    return FALSE;
  }

  return TRUE;
}

static void
hud_julius_kill(GPid pid)
{
  kill(pid, SIGTERM);
}

static gboolean
hud_julius_listen (HudJulius *self, const gchar *gram, const gchar *hmm,
    const gchar *hlist, gchar **query, GError **error)
{
  gchar *program = g_build_filename(LIBEXECDIR, "hud", "hud-julius-listen", NULL);
  g_debug("Julius listening program [%s]", program);

  const gchar *argv[] =
  { program, "-input", "pulseaudio", "-gram", gram, "-h", hmm, "-hlist", hlist, NULL };

  /* These are used inside the callbacks */
  self->error = error;
  self->query = query;
  self->listen_emitted = FALSE;
  self->heard_something_emitted = FALSE;

  gint standard_output;
  GPid pid = 0;

  if (!g_spawn_async_with_pipes (NULL, (gchar **) argv, NULL, 0, NULL, NULL,
      &pid, NULL, &standard_output, NULL, error))
  {
    g_warning("Failed to to load Julius daemon");
    *query = NULL;
    *error = g_error_new_literal (hud_julius_error_quark (), 0,
        "Failed to load Julius daemon");
    g_free(program);
    return FALSE;
  }

  g_free(program);

  GIOChannel *channel;

  channel = g_io_channel_unix_new (standard_output);
  g_io_channel_set_flags (channel,
      g_io_channel_get_flags (channel) | G_IO_FLAG_NONBLOCK, NULL );
  g_io_channel_set_encoding (channel, NULL, NULL );

  self->watch_source = g_io_add_watch (channel,
      G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL,
               watch_function,
               self);

  self->mainloop = g_main_loop_new (NULL, FALSE);
  self->timeout_source = g_timeout_add (HUD_JULIUS_DEFAULT_TIMEOUT / 1000, timeout_quit_func, self);
  g_main_loop_run (self->mainloop);
  g_main_loop_unref (self->mainloop);

  g_io_channel_unref (channel);
  hud_julius_kill(pid);
  g_spawn_close_pid(pid);

  return (*self->error == NULL);
}

static void
free_func (gpointer data)
{
  g_ptr_array_free((GPtrArray*) data, TRUE);
}

static const gchar *VOCA_HEADER = "% NS_B\n"
"<s>             sil\n"
"\n"
"% NS_E\n"
"</s>            sil\n"
"\n";

static gboolean
hud_julius_build_grammar (HudJulius *self, GList *items, gchar **temp_dir, GError **error)
{
  *temp_dir = g_dir_make_tmp ("hud-julius-XXXXXX", error);
  if (*temp_dir == NULL )
  {
    g_warning("Failed to create temp dir [%s]", (*error)->message);
    return FALSE;
  }

  PronounceDict *dict = pronounce_dict_get_julius(error);
  if (dict == NULL)
  {
    rm_rf(*temp_dir);
    g_clear_pointer(temp_dir, g_free);
    return FALSE;
  }

  /* Get the pronounciations for the items */
  GHashTable *pronounciations = g_hash_table_new_full (g_str_hash, g_str_equal,
      g_free, (GDestroyNotify) g_strfreev);
  GPtrArray *command_list = g_ptr_array_new_with_free_func (free_func);
  HudItemPronunciationData pronounciation_data =
  { pronounciations, self->alphanumeric_regex, command_list, dict };
  g_list_foreach (items, (GFunc) hud_item_insert_pronounciation,
      &pronounciation_data);

  if (command_list->len == 0)
  {
    *error = g_error_new_literal(hud_julius_error_quark(), 0, "Could not build Julius grammar. Is julius-voxforge installed?");
    rm_rf(*temp_dir);
    g_clear_pointer(temp_dir, g_free);
    return FALSE;
  }

  gint voca_id_counter = 0;
  GHashTable *voca = g_hash_table_new_full (g_str_hash, g_str_equal, g_free,
      NULL );

  gchar *voca_path = g_build_filename (*temp_dir, "hud.voca", NULL );
  GFile *voca_file = g_file_new_for_path (voca_path);
  error = NULL;
  GOutputStream* voca_output = G_OUTPUT_STREAM(g_file_create (voca_file,
          G_FILE_CREATE_PRIVATE, NULL, error ));
  if (voca_output == NULL )
  {
    g_warning("Failed to open voca file [%s]", (*error)->message);

    g_hash_table_destroy(voca);
    g_hash_table_destroy(pronounciations);
    g_ptr_array_free (command_list, TRUE);

    g_object_unref (voca_output);
    g_object_unref (voca_file);

    rm_rf(*temp_dir);
    g_clear_pointer(temp_dir, g_free);

    return FALSE;
  }

  gchar *grammar_path = g_build_filename (*temp_dir, "hud.grammar", NULL );
  GFile *grammar_file = g_file_new_for_path (grammar_path);
  error = NULL;
  GOutputStream* grammar_output = G_OUTPUT_STREAM(g_file_create (grammar_file,
          G_FILE_CREATE_PRIVATE, NULL, error ));
  if (grammar_output == NULL )
  {
    g_warning("Failed to open grammar file [%s]", (*error)->message);

    g_hash_table_destroy(voca);
    g_hash_table_destroy(pronounciations);
    g_ptr_array_free (command_list, TRUE);

    g_object_unref (voca_output);
    g_object_unref (voca_file);

    g_object_unref (grammar_output);
    g_object_unref (grammar_file);

    rm_rf(*temp_dir);
    g_clear_pointer(temp_dir, g_free);

    return FALSE;
  }

  g_output_stream_write (voca_output, VOCA_HEADER,
      g_utf8_strlen (VOCA_HEADER, -1), NULL, NULL );

  guint i, j;
  for (i = 0; i < command_list->len; ++i)
  {
    GPtrArray *command = g_ptr_array_index(command_list, i);

    g_output_stream_write (grammar_output, "S : NS_B ", g_utf8_strlen ("S : NS_B ", -1),
                NULL, NULL );

      for (j = 0; j < command->len; ++j)
      {
        const gchar *word = g_ptr_array_index(command,  j);
        gint voca_id = GPOINTER_TO_INT(g_hash_table_lookup(voca, word));

        /* If this a new phonetic */
        if (voca_id == 0)
        {
          voca_id = ++voca_id_counter;
          g_hash_table_insert(voca, g_strdup(word), GINT_TO_POINTER(voca_id));

          gchar *voca_id_str = g_strdup_printf("TOKEN_%d", voca_id);
          gchar **phonetics_list = g_hash_table_lookup(pronounciations, word);

          /* We are writing:
           *
           * % <voca_id>:
           * <word> <phonetics 1>
           * <word> <phonetics 2>
           */

          /* we only need to write out the vocab entry for a new token */
          g_output_stream_write (voca_output, "% ", g_utf8_strlen ("% ", -1),
                                      NULL, NULL );
          g_output_stream_write (voca_output, voca_id_str,
                        g_utf8_strlen (voca_id_str, -1), NULL, NULL );
          g_output_stream_write (voca_output, ":\n", g_utf8_strlen (":\n", -1),
                              NULL, NULL );

          gchar **phonetics = phonetics_list;
          while (*phonetics)
          {
            gchar* lower = g_utf8_strdown(*phonetics, -1);

            g_output_stream_write (voca_output, word,
                    g_utf8_strlen (word, -1), NULL, NULL );
            g_output_stream_write (voca_output, "\t",
                            g_utf8_strlen ("\t", -1), NULL, NULL );
            /* also add the phonetics */
            g_output_stream_write (voca_output, lower,
                                    g_utf8_strlen (lower, -1), NULL, NULL );
            g_output_stream_write (voca_output, "\n", g_utf8_strlen ("\n", -1),
                          NULL, NULL );
            g_free(lower);
            phonetics++;
          }

          g_output_stream_write (voca_output, "\n", g_utf8_strlen ("\n", -1),
            NULL, NULL );

          g_free(voca_id_str);
        }

        gchar *voca_id_str = g_strdup_printf("TOKEN_%d", voca_id);

        g_output_stream_write (grammar_output, voca_id_str,
            g_utf8_strlen (voca_id_str, -1), NULL, NULL );
        g_output_stream_write (grammar_output, " ", g_utf8_strlen (" ", -1),
                      NULL, NULL );
        g_free(voca_id_str);
      }

      g_output_stream_write (grammar_output, " NS_E\n",
          g_utf8_strlen (" NS_E\n", -1), NULL, NULL );
    }

  g_hash_table_destroy(voca);
  g_hash_table_destroy(pronounciations);
  g_ptr_array_free (command_list, TRUE);

  g_object_unref (voca_output);
  g_object_unref (voca_file);

  g_object_unref (grammar_output);
  g_object_unref (grammar_file);

  g_free(voca_path);
  g_free(grammar_path);

  /* Now compile the grammar */
  const gchar *argv[] = { "/usr/bin/mkdfa", "hud", NULL };
  error = NULL;
  int exit_status = 0;
  char *standard_output = NULL;
  char *standard_error = NULL;
  if (!g_spawn_sync (*temp_dir, (gchar **)argv, NULL, 0, NULL, NULL, &standard_output,
      &standard_error, &exit_status, error))
  {
    g_warning("Compiling grammar failed: [%s]", (*error)->message);
    g_debug("Compilation errors:\n%s", standard_error);

    g_free(standard_output);
    g_free(standard_error);

    rm_rf(*temp_dir);
    g_clear_pointer(temp_dir, g_free);

    return FALSE;
  }

  g_debug("Compilation output:\n%s", standard_output);

  g_free(standard_output);
  g_free(standard_error);

  return TRUE;
}

static void rm_rf(const gchar *path)
{
  GError *error = NULL;
  GDir *dir = g_dir_open(path, 0, &error);
  if (dir == NULL)
  {
    g_warning("Could not open directory [%s] for deletion: [%s]", path, error->message);
    g_error_free(error);
    return;
  }
  const gchar *file_name;
  while ((file_name = g_dir_read_name(dir)))
  {
    gchar *file_path = g_build_filename(path, file_name, NULL);
    g_debug("removing file [%s]", file_path);
    g_remove(file_path);
    g_free(file_path);
  }
  g_dir_close(dir);
  g_debug("removing dir [%s]", path);
  g_rmdir(path);
}

gboolean
hud_julius_voice_query (HudJulius *self, HudSource *source, gchar **result, GError **error)
{
  if (source == NULL) {
    /* No active window, that's fine, but we'll just move on */
    *result = NULL;
    *error = g_error_new_literal(hud_julius_error_quark(), 0, "Active source is null");
    return FALSE;
  }

  GList *items = hud_source_get_items(source);
  if (items == NULL) {
    /* The active window doesn't have items, that's cool.  We'll move on. */
    return TRUE;
  }

  gchar *temp_dir = NULL;
  if (!hud_julius_build_grammar(self, items, &temp_dir, error))
  {
    return FALSE;
  }

  gchar *gram = g_build_filename(temp_dir, "hud", NULL);
  gchar *hmm = g_build_filename(JULIUS_DICT_PATH, "hmmdefs", NULL);
  gchar *hlist = g_build_filename(JULIUS_DICT_PATH, "tiedlist", NULL);

  gboolean success = hud_julius_listen (self, gram, hmm, hlist, result, error);

  rm_rf(temp_dir);

  g_free(gram);
  g_free(hmm);
  g_free(hlist);
  g_free(temp_dir);

  return success;
}

HudJulius *
hud_julius_new(HudQueryIfaceComCanonicalHudQuery * skel)
{
  HudJulius *self = g_object_new (HUD_TYPE_JULIUS, NULL);
  self->skel = g_object_ref(skel);

  return self;
}
