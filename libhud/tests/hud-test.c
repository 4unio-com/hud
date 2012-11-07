
#include <glib-object.h>


static void
test_suite (void)
{

	return;
}

int
main (int argc, char * argv[])
{
	g_type_init();
	g_test_init(&argc, &argv, NULL);

	/* Test Suites */
	test_suite();

	return g_test_run();
}
