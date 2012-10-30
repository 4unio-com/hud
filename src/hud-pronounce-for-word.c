
#include <pronounce-dict.h>

int
main (int argc, char * argv[])
{
	if (argc != 2) {
		return 1;
	}

	g_type_init();

	PronounceDict * dict = g_object_new(PRONOUNCE_DICT_TYPE, NULL);

	gchar * pronounce = pronounce_dict_lookup_word(dict, argv[1]);

	g_print("%s:\t%s\n", argv[1], pronounce);
	g_free(pronounce);

	g_clear_object(&dict);

	return 0;
}
