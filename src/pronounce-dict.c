#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pronounce-dict.h"

#include <gio/gio.h>

struct _PronounceDictPrivate {
	GHashTable * dict;
};

#define PRONOUNCE_DICT_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), PRONOUNCE_DICT_TYPE, PronounceDictPrivate))

static void pronounce_dict_class_init (PronounceDictClass *klass);
static void pronounce_dict_init       (PronounceDict *self);
static void pronounce_dict_dispose    (GObject *object);
static void pronounce_dict_finalize   (GObject *object);
static gboolean load_dict                 (PronounceDict * dict, const gchar *dict_path, GError **error);

G_DEFINE_TYPE (PronounceDict, pronounce_dict, G_TYPE_OBJECT);

/* Build up our class */
static void
pronounce_dict_class_init (PronounceDictClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (PronounceDictPrivate));

	object_class->dispose = pronounce_dict_dispose;
	object_class->finalize = pronounce_dict_finalize;

	return;
}

static GQuark
hud_pronounce_dict_error_quark(void)
{
  static GQuark quark = 0;
  if (quark == 0)
    quark = g_quark_from_static_string ("hud-pronounce-dict-error-quark");
  return quark;
}

/* We've got a list of strings, let's free it */
static void
str_list_free (gpointer data)
{
	GList * list = (GList *)data;
	g_list_free_full(list, g_free);

	return;
}

/* Initialize stuff */
static void
pronounce_dict_init (PronounceDict *self)
{
	self->priv = PRONOUNCE_DICT_GET_PRIVATE(self);

	self->priv->dict = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, str_list_free);
}

/* Drop references */
static void
pronounce_dict_dispose (GObject *object)
{

	G_OBJECT_CLASS (pronounce_dict_parent_class)->dispose (object);
	return;
}

/* Clean up memory */
static void
pronounce_dict_finalize (GObject *object)
{
	PronounceDict * dict = PRONOUNCE_DICT(object);

	g_hash_table_unref(dict->priv->dict);

	G_OBJECT_CLASS (pronounce_dict_parent_class)->finalize (object);
	return;
}

/* Load the dictionary from a file */
static gboolean
load_dict (PronounceDict *dict, const gchar *dict_path, GError **error)
{
	if (!g_file_test(dict_path, G_FILE_TEST_EXISTS)) {
		g_warning("Unable to find dictionary '%s'!", dict_path);
		if (error != NULL)
          *error = g_error_new(hud_pronounce_dict_error_quark (), 0, "Unable to find dictionary [%s].", dict_path);
		return FALSE;
	}

	GFile * dict_file = g_file_new_for_path(dict_path);
	GFileInputStream * stream = g_file_read(dict_file, NULL, NULL);
	GDataInputStream * dstream = g_data_input_stream_new(G_INPUT_STREAM(stream));

	gchar * line = NULL;
	while ((line = g_data_input_stream_read_line(dstream, NULL, NULL, NULL)) != NULL) {
		/* Catch the NULL string and just kill it early */
		if (line[0] == '\0') {
			g_free(line);
			continue;
		}

		if (line[0] == ';' && line[1] == ';' && line[2] == ';') {
			g_free(line);
			continue;
		}

		if (line[0] == '#') {
			g_free(line);
			continue;
		}

		gchar ** split = NULL;

		gchar *htk_start = g_strrstr(line, "[");
		gchar *htk_end = g_strrstr(line, "]");

		/* If we have the HTK style dict */
		if (htk_start != NULL && htk_end != NULL)
		{
		  GArray *split_array = g_array_sized_new(TRUE, FALSE, sizeof(gchar *), 2);

		  gchar *first = g_strndup(line, htk_start - line);
		  first = g_strchomp(first);
		  g_array_append_val(split_array, first);

		  gchar *second = g_strdup(htk_end + 1);
      g_array_append_val(split_array, second);

      split = (gchar **) g_array_free(split_array, FALSE);
		}
		else
		{
      /* Break it on the tab so that we have the name and the phonetics
         broken apart */
      split = g_strsplit_set(line, " \t", 2);
      if (split[0] == NULL) {
        g_strfreev(split);
        continue;
      }
	  }

		gchar * word = g_utf8_strup(split[0], -1);
		gchar * phonetics = g_strstrip(split[1]);

		gunichar charone = g_utf8_get_char(word);

		/* Handle those that are punctuation stuff */
		if (!g_unichar_isalnum(charone)) {
			gchar * lookingchar = word + 1;

			while (lookingchar != NULL && !g_unichar_isalnum(g_utf8_get_char(lookingchar))) {
				lookingchar++;
			}

			if (lookingchar != NULL) {
				lookingchar[0] = '\0';
			}
		}

		/* Some words end with a counter, which is nice, but we don't care */
		if (g_str_has_suffix(word, ")")) {
			/* Magic +1 to make sure it isn't the first character, so the description
			   for the character */
			gchar * first_paren = g_strrstr(word + 1, "(");
			if (first_paren != NULL) {
				first_paren[0] = '\0';
			}
		}

		/* This will be NULL if it doesn't exist, which GList handles
		   nicely for us -- cool, little trick, eh? */
		GList * phono_list = g_list_copy_deep((GList *)g_hash_table_lookup(dict->priv->dict, word), (GCopyFunc)g_strdup, NULL);

		if (phonetics != NULL) {
			gchar ** splitted = g_strsplit(phonetics, " ", -1);
			g_free(phonetics);

			gint i;
			for (i = 0; splitted[i] != NULL; i++) {
				gint len = g_utf8_strlen(splitted[i], -1);

				if (g_unichar_isdigit(g_utf8_get_char(&splitted[i][len - 1]))) {
					splitted[i][len - 1] = '\0';
				}
			}

			phonetics = g_strjoinv(" ", splitted);
			split[1] = phonetics;

			g_strfreev(splitted);
		}

		phono_list = g_list_append(phono_list, g_strdup(phonetics));
		g_hash_table_insert(dict->priv->dict, g_strdup(word), phono_list);

		g_free(word);
		g_strfreev(split);
		g_free(line);
	}

	g_free(line);

  g_object_unref(dstream);
  g_object_unref(stream);
  g_object_unref(dict_file);

	return TRUE;
}

/* Puts the results of a lookup into an array.  Assumes the word is already
   upper case an a bunch of other fun things. */
static void
pronounce_dict_lookup_word_internal (PronounceDict * dict, gchar * word, GArray * results)
{
	if (word[0] == '\0') {
		return;
	}

	GList * retval = (GList *)g_hash_table_lookup(dict->priv->dict, word);
	if (retval != NULL) {
		while (retval != NULL) {
			gchar * appval = g_strdup((gchar *)retval->data);
			g_array_append_val(results, appval);
			retval = retval->next;
		}
	}
}

/* Lookup a word and return results */
gchar **
pronounce_dict_lookup_word(PronounceDict * dict, gchar * word)
{
	g_return_val_if_fail(IS_PRONOUNCE_DICT(dict), NULL);

	GArray * results = g_array_new(TRUE, FALSE, sizeof(gchar *));
	gchar * upper = g_utf8_strup(word, -1);

	pronounce_dict_lookup_word_internal(dict, upper, results);

	g_free(upper);

	return (gchar **) g_array_free(results, FALSE);
}

PronounceDict *
pronounce_dict_new (const gchar *dict_path, GError **error)
{
  PronounceDict *dict = g_object_new(PRONOUNCE_DICT_TYPE, NULL);
  if (!load_dict(dict, dict_path, error))
  {
    g_clear_object(&dict);
  }
  return dict;
}

PronounceDict *
pronounce_dict_get_sphinx (GError **error)
{
	static PronounceDict * global = NULL;

	if (global == NULL) {
	  global = pronounce_dict_new(DICT_PATH, error);
	}

	return global;
}
