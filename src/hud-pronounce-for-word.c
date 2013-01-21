
#include <pronounce-dict.h>

int
main (int argc, char * argv[])
{
	if (argc != 2) {
		return 1;
	}

	g_type_init();

	PronounceDict * dict = g_object_new(PRONOUNCE_DICT_TYPE, NULL);

	gchar ** pronounce = pronounce_dict_lookup_word(dict, argv[1]);
	gchar * pron = NULL;
	int i;

	for (i = 0; (pron = pronounce[i]) != NULL; i++) {
		g_print("%s:\t%s\n", argv[1], pron);
	}

	g_strfreev(pronounce);

	g_clear_object(&dict);

	return 0;
}
