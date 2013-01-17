
#include <glib-object.h>


static void
test_suite (void)
{

	return;
}

int
main (int argc, char * argv[])
{
	if (!GLIB_CHECK_VERSION(2, 35, 0))
		g_type_init (); /* Only needed in versions < 2.35.0 */

	g_test_init(&argc, &argv, NULL);

	/* Test Suites */
	test_suite();

	return g_test_run();
}
