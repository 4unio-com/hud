include $(top_srcdir)/gtk-doc.make

DOC_MODULE = libhud-client

DOC_MAIN_SGML_FILE = $(DOC_MODULE)-docs.xml

DOC_SOURCE_DIR = $(top_srcdir)/libhud-client

MKDB_OPTIONS = --sgml-mode --output-format=xml

HFILE_GLOB = $(DOC_SOURCE_DIR)/*.h
CFILE_GLOB = $(DOC_SOURCE_DIR)/*.c

IGNORE_HFILES = \
	query-iface.h \
	service-iface.h

IGNORE_CFILES = \
	query-iface.c \
	service-iface.c

GTKDOC_CFLAGS = -I$(top_srcdir)/libhud-client -I$(top_srcdir) $(HUD_CFLAGS)

GTKDOC_LIBS = \
	$(top_builddir)/libhud-client/libhud-client.la \
	$(HUD_LIBS)

TESTS = gtkdoc-in-srcdir

gtkdoc-in-srcdir: Makefile.am
	@echo "#!/bin/sh" > $@
	@echo "cd \"$(srcdir)\"" >> $@
	@echo "$(GTKDOC_CHECK)" >> $@
	@chmod +x $@

DISTCLEANFILES = gtkdoc-in-srcdir

