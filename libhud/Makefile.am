
lib_LTLIBRARIES = libhud.la

ABI_VERSION = 1
API_VERSION = 1

libhudincludedir = $(includedir)/libhud-$(API_VERSION)/libhud
libhudinclude_HEADERS = \
	hud.h

libhud_la_SOURCES = \
	hud-service-iface.h \
	hud-service-iface.c \
	hud.h \
	hud.c

libhud_la_CFLAGS = \
	$(HUD_CFLAGS) \
	$(COVERAGE_CFLAGS) \
	-I $(top_srcdir) \
	-D G_LOG_DOMAIN=\"libhud\" \
	-Wall -Werror

libhud_la_LIBADD = \
	$(HUD_LIBS)

libhud_la_LDFLAGS = \
	$(HUD_LDFLAGS) \
	$(COVERAGE_LDFLAGS) \
	-version-info $(ABI_VERSION):0:0 \
	-no-undefined \
	-export-symbols-regex "^[^_].*"

hud-service-iface.c: $(top_srcdir)/data/com.canonical.hud.Application.xml
	$(AM_V_GEN) gdbus-codegen \
		--interface-prefix com.canonical.hud.Application. \
		--generate-c-code hud-service-iface \
		--c-namespace _HudService \
		$^
hud-service-iface.h: hud-service-iface.c

BUILT_SOURCES = \
	hud-service-iface.h \
	hud-service-iface.c

EXTRA_DIST = \
	hud.pc.in

pkgconfig_DATA = hud-$(API_VERSION).pc
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
	hud-$(API_VERSION).pc

