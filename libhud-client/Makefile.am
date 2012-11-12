
# SUBDIRS = tests

lib_LTLIBRARIES = libhud-client.la

ABI_VERSION = 1
API_VERSION = 1

libhud_client_includedir = $(includedir)/libhud-client-$(API_VERSION)/libhud
libhud_client_include_HEADERS = \
	hud-client.h

libhud_client_la_SOURCES = \
	hud-service-iface.h \
	hud-service-iface.c \
	hud-client-connection.h \
	hud-client-connection.c
	hud-client.h \
	hud-client.c

libhud_client_la_CFLAGS = \
	$(HUD_CFLAGS) \
	$(COVERAGE_CFLAGS) \
	-I $(top_srcdir) \
	-D G_LOG_DOMAIN=\"libhud-client\" \
	-Wall -Werror

libhud_client_la_LIBADD = \
	$(HUD_LIBS)

libhud_client_la_LDFLAGS = \
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
	hud-client.pc.in

pkgconfig_DATA = hud-client-$(API_VERSION).pc
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
	hud-client-$(API_VERSION).pc

