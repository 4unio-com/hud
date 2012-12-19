
SUBDIRS = .

lib_LTLIBRARIES = libhud-gtk.la

ABI_VERSION = 1
API_VERSION = 1

libhud_gtk_include_basedir = $(includedir)/libhud-gtk-$(API_VERSION)/
libhud_gtk_include_base_HEADERS = \
	hud-gtk.h

libhud_gtkincludedir = $(includedir)/libhud-gtk-$(API_VERSION)/libhud-gtk
libhud_gtkinclude_HEADERS = \
	manager.h

libhud_gtk_la_SOURCES = \
	manager.c

libhud_gtk_la_CFLAGS = \
	$(HUD_CFLAGS) \
	$(COVERAGE_CFLAGS) \
	-I . \
	-I $(top_srcdir) \
	-I $(top_srcdir)/libhud \
	-D G_LOG_DOMAIN=\"libhud-gtk\" \
	-D HUD_GTK_COMPILATION \
	-Wall -Werror

libhud_gtk_la_LIBADD = \
	$(HUD_LIBS)

libhud_gtk_la_LDFLAGS = \
	$(HUD_LDFLAGS) \
	$(COVERAGE_LDFLAGS) \
	-version-info $(ABI_VERSION):0:0 \
	-no-undefined \
	-export-symbols-regex "^[^_].*"

EXTRA_DIST = \
	hud-gtk.pc.in

pkgconfig_DATA = hud-gtk-$(API_VERSION).pc
pkgconfigdir = $(libdir)/pkgconfig

%-$(API_VERSION).pc: %.pc.in
	sed \
		-e "s:\@apiversion\@:$(API_VERSION):" \
		-e "s:\@libdir\@:$(libdir):" \
		-e "s:\@includedir\@:$(includedir):" \
		-e "s:\@VERSION\@:$(VERSION):" \
		$< > $@

CLEANFILES = \
	$(BUILD_SOURCES) \
	hud-gtk-$(API_VERSION).pc

