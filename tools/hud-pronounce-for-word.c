
#include <pronounce-dict.h>

int
main (int argc, char * argv[])
{
	if (argc != 2) {
		return 1;
	}

#ifndef GLIB_VERSION_2_36
	g_type_init();
#endif

	GError *error = NULL;
	PronounceDict * dict = pronounce_dict_get_sphinx(&error);
	if (dict == NULL)
	{
	  g_error("%s", error->message);
	  g_error_free(error);
	  return 1;
	}

	gchar ** pronounce = pronounce_dict_lookup_word(dict, argv[1]);
	gchar * pron = NULL;
	int i;

	for (i = 0; (pron = pronounce[i]) != NULL; i++) {
		g_print("%s:\t%s\n", argv[1], pron);
	}
	if (i == 0) {
	  g_message("No pronunciations");
	}

	g_strfreev(pronounce);

	g_clear_object(&dict);

	return 0;
}
