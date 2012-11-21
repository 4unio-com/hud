
CLEANFILES =
DISTCLEANFILES =
EXTRA_DIST =
check_PROGRAMS =

TESTS = \
	libhud-client-tester

#####################################
# libhud-client-test
#####################################

LIBHUD_XML_REPORT = libhud-client-test.xml
libhud-client-tester: libhud-client-test Makefile.am
	@echo "#!/bin/bash" > $@
	@echo gtester --verbose -k -o $(LIBHUD_XML_REPORT) $(abs_builddir)/libhud-client-test >> $@
	@chmod +x $@

CLEANFILES += \
	$(LIBHUD_XML_REPORT) \
	libhud-client-tester

libhud_client_test_SOURCES = \
	hud-client-test.c
libhud_client_test_CFLAGS = \
	-D HUD_SERVICE="\"$(top_builddir)/src/hud-service\"" \
	-I $(top_srcdir) \
	-I $(top_srcdir)/libhud-client \
	$(HUD_CFLAGS) \
	$(DBUSTEST_CFLAGS) \
	$(TEST_CFLAGS)
libhud_client_test_LDADD = \
	$(top_builddir)/libhud-client/libhud-client.la \
	$(HUD_LIBS) \
	$(DBUSTEST_LIBS) \
	$(TEST_LIBS)

check_PROGRAMS += libhud-client-test

