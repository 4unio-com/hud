check_PROGRAMS = \
	simple-test

simple_test_CFLAGS = \
	$(HUD_CFLAGS) \
	$(gtk_CFLAGS) \
	-I$(top_srcdir) \
	-I$(top_srcdir)/libhud

simple_test_LDADD = \
	$(top_builddir)/libhud/libhud.la \
	$(HUD_LIBS) \
	$(gtk_LIBS)
