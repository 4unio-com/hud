
CLEANFILES =
DISTCLEANFILES =
EXTRA_DIST =
check_PROGRAMS =

TESTS = \
	libhud-tester

#####################################
# libhud-test
#####################################

LIBHUD_XML_REPORT = libhud-test.xml
libhud-tester: libhud-test Makefile.am
	@echo "#!/bin/bash" > $@
	@echo gtester --verbose -k -o $(LIBHUD_XML_REPORT) $(abs_builddir)/libhud-test >> $@
	@chmod +x $@

CLEANFILES += \
	$(LIBHUD_XML_REPORT) \
	libhud-tester

libhud_test_SOURCES = \
	hud-test.c
libhud_test_CFLAGS = \
	-I $(top_srcdir) \
	$(HUD_CFLAGS) \
	$(TEST_CFLAGS)
libhud_test_LDADD = \
	$(builddir)/../libhud.la \
	$(HUD_LIBS) \
	$(TEST_LIBS)

check_PROGRAMS += libhud-test

