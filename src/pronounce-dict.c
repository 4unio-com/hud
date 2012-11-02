#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pronounce-dict.h"

#include <gio/gio.h>

#define DICT_PATH "/home/ted/Development/cmusphinx/trunk/cmudict/cmudict.0.7a"

struct _PronounceDictPrivate {
	GHashTable * dict;
};

#define PRONOUNCE_DICT_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), PRONOUNCE_DICT_TYPE, PronounceDictPrivate))

static void pronounce_dict_class_init (PronounceDictClass *klass);
static void pronounce_dict_init       (PronounceDict *self);
static void pronounce_dict_dispose    (GObject *object);
static void pronounce_dict_finalize   (GObject *object);
static void load_dict                 (PronounceDict * dict);

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

	load_dict(self);

	return;
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
static void
load_dict (PronounceDict * dict)
{
	if (!g_file_test(DICT_PATH, G_FILE_TEST_EXISTS)) {
		g_warning("Unable to find dictionary '%s'!", DICT_PATH);
		return;
	}

	GFile * dict_file = g_file_new_for_path(DICT_PATH);
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

		/* Break it on the tab so that we have the name and the phonetics
		   broken apart */
		gchar ** split = g_strsplit(line, "  ", 2);
		if (split[0] == NULL) {
			g_strfreev(split);
			continue;
		}

		gchar * word = split[0];
		gchar * phonetics = split[1];

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

		g_strfreev(split);
		g_free(line);
	}

	g_free(line);

	return;
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
		return;
	}

	glong fullstring = g_utf8_strlen(word, -1);
	gchar * frontstring = g_utf8_substring(word, 0, fullstring - 1);
	GArray * front_array = g_array_new(TRUE, FALSE, sizeof(gchar *));
	pronounce_dict_lookup_word_internal(dict, frontstring, front_array);

	g_free(frontstring);

	if (front_array->len == 0) { /* It is nowhere... which seems odd */
		g_array_free(front_array, TRUE);
		return;
	}

	gchar * laststring = g_utf8_substring(word, fullstring - 1, fullstring);
	GArray * last_array = g_array_new(TRUE, FALSE, sizeof(gchar *));
	pronounce_dict_lookup_word_internal(dict, laststring, last_array);

	g_free(laststring);

	if (last_array->len == 0) {
		g_array_free(front_array, TRUE);
		g_array_free(last_array, TRUE);
		return;
	}

	int i, j;
	for (i = 0; i < front_array->len; i++) {
	for (j = 0; j < last_array->len; j++) {
		gchar * a = g_array_index(front_array, gchar *, i);
		gchar * b = g_array_index(last_array, gchar *, j);

		gchar * output = NULL;
		if (a[0] != '\0' && b[0] != '\0') {
			output = g_strdup_printf("%s %s", g_array_index(front_array, gchar *, i), g_array_index(last_array, gchar *, j));
		} else {
			if (a[0] != '\0') {
				output = g_strdup(a);
			} else {
				output = g_strdup(b);
			}
		}

		g_array_append_val(results, output);
	} // j
	} // i

	g_array_free(front_array, TRUE);
	g_array_free(last_array, TRUE);

	return;
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
pronounce_dict_get (void)
{
	static PronounceDict * global = NULL;

	if (global == NULL) {
		global = g_object_new(PRONOUNCE_DICT_TYPE, NULL);
	}

	return global;
}
