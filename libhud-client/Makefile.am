
SUBDIRS = tests

lib_LTLIBRARIES = libhud-client.la

ABI_VERSION = 1
API_VERSION = 1

libhud_client_include_basedir = $(includedir)/libhud-client-$(API_VERSION)/
libhud_client_include_base_HEADERS = \
	hud-client.h

libhud_client_includedir = $(includedir)/libhud-client-$(API_VERSION)/libhud-client
libhud_client_include_HEADERS = \
	connection.h \
	query.h

libhud_client_la_SOURCES = \
	service-iface.h \
	service-iface.c \
	query-iface.h \
	query-iface.c \
	connection.h \
	connection.c \
	query.h \
	query.c \
	hud-client.h

libhud_client_la_CFLAGS = \
	$(HUD_CFLAGS) \
	$(COVERAGE_CFLAGS) \
	-I $(top_srcdir) \
	-I $(top_srcdir)/src \
	-D G_LOG_DOMAIN=\"libhud-client\" \
	-D HUD_CLIENT_COMPILATION \
	-Wall -Werror

libhud_client_la_LIBADD = \
	$(HUD_LIBS)

libhud_client_la_LDFLAGS = \
	$(HUD_LDFLAGS) \
	$(COVERAGE_LDFLAGS) \
	-version-info $(ABI_VERSION):0:0 \
	-no-undefined \
	-export-symbols-regex "^[^_].*"

service-iface.c: $(top_srcdir)/data/com.canonical.hud.xml
	$(AM_V_GEN) gdbus-codegen \
		--interface-prefix com.canonical.hud. \
		--generate-c-code service-iface \
		--c-namespace _HudService \
		$^
service-iface.h: service-iface.c

query-iface.c: $(top_srcdir)/data/com.canonical.hud.query.xml
	$(AM_V_GEN) gdbus-codegen \
		--interface-prefix com.canonical.hud.query. \
		--generate-c-code query-iface \
		--c-namespace _HudQuery \
		$^
query-iface.h: query-iface.c

BUILT_SOURCES = \
	query-iface.h \
	query-iface.c \
	service-iface.h \
	service-iface.c

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

