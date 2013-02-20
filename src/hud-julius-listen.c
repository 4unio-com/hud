/*
 Copyright 2011 Canonical Ltd.

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
#include <julius/juliuslib.h>

static void
hud_julius_listen_listening (Recog *recog, void *user_data)
{
  g_print ("<voice-query-listening/>\n");
}

static void
hud_julius_listen_heard_something (Recog *recog, void *user_data)
{
  g_print ("<voice-query-heard-something/>\n");
}

static void
hud_julius_listen_output (Recog *recog, void *user_data)
{
  int i;
  WORD_INFO *winfo;
  WORD_ID *seq;
  int seqnum;
  int n;
  Sentence *s;
  RecogProcess *r;

  for (r = recog->process_list; r; r = r->next)
  {
    if (!r->live)
      continue;

    if (r->result.status < 0)
    {
      switch (r->result.status)
      {
      case J_RESULT_STATUS_REJECT_POWER:
        g_warning("input rejected by power");
        break;
      case J_RESULT_STATUS_TERMINATE:
        g_warning("input teminated by request");
        break;
      case J_RESULT_STATUS_ONLY_SILENCE:
        g_warning("input rejected by decoder (silence input result)");
        break;
      case J_RESULT_STATUS_REJECT_GMM:
        g_warning("input rejected by GMM");
        break;
      case J_RESULT_STATUS_REJECT_SHORT:
        g_warning("input rejected by short input");
        break;
      case J_RESULT_STATUS_FAIL:
        g_warning("search failed");
        break;
      }
      continue;
    }

    winfo = r->lm->winfo;

    for (n = 0; n < r->result.sentnum; n++)
    {
      s = &(r->result.sent[n]);
      seq = s->word;
      seqnum = s->word_num;

      GArray *words = g_array_sized_new(TRUE, FALSE, sizeof(gchar *), seqnum);
      for (i = 0; i < seqnum; i++)
      {
        const gchar* word = winfo->woutput[seq[i]];
        if (g_strcmp0(word, "<s>") == 0 || g_strcmp0(word, "</s>") == 0)
          continue;
        gchar *copy = g_strdup(word);
        g_array_append_val(words, copy);
      }
      gchar **result = (gchar **) g_array_free(words, FALSE);
      gchar *output = g_strjoinv(" ", result);
      g_print ("<voice-query-finished>%s</voice-query-finished>\n", output);
      g_free(output);
      g_strfreev(result);
    }
  }

}

int main (int argc, char *argv[])
{
  Jconf *jconf;
  Recog *recog;

  if (argc == 1)
  {
    g_error("hud-julius-listen - use regular Julius arguments");
    return -1;
  }

  jlog_set_output(NULL);

  jconf = j_config_load_args_new (argc, argv);
  if (jconf == NULL )
  {
    g_error("Try `-help' for more information");
    return -1;
  }

  recog = j_create_instance_from_jconf (jconf);
  if (recog == NULL )
  {
    g_error("Error in startup");
    return -1;
  }

  callback_add (recog, CALLBACK_EVENT_SPEECH_READY, hud_julius_listen_listening,
      NULL );
  callback_add (recog, CALLBACK_EVENT_SPEECH_START,
      hud_julius_listen_heard_something, NULL );
  callback_add (recog, CALLBACK_RESULT, hud_julius_listen_output, NULL );

  if (j_adin_init (recog) == FALSE)
  {
    return -1;
  }

  switch (j_open_stream (recog, NULL ))
  {
  case 0:
    break;
  case -1:
    g_error("error in input stream");
    return -1;
  case -2:
    g_error("failed to begin input stream");
    return -1;
  }

  if (j_recognize_stream (recog) == -1)
    return -1;

  j_close_stream (recog);
  j_recog_free (recog);
  return 0;
}
