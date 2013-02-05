
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
	param.h \
	query.h

noinst_LTLIBRARIES = libhud-client-generated.la

libhud_client_generated_la_SOURCES = \
	service-iface.h \
	service-iface.c \
	query-iface.h \
	query-iface.c

libhud_client_generated_la_CFLAGS = \
	$(HUD_CFLAGS) \
	-Wall -Werror

libhud_client_generated_la_LIBADD = \
	$(HUD_LIBS)

libhud_client_la_SOURCES = \
	connection.h \
	connection.c \
	param.h \
	param.c \
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
	libhud-client-generated.la \
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

##########################
# Introspection
##########################

include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ARGS = \
	--add-include-path=$(top_srcdir) \
	--add-include-path=$(srcdir) \
	-I $(top_srcdir) -I $(srcdir)
INTROSPECTION_COMPILER_ARGS = --includedir=$(top_srcdir) --includedir=$(srcdir)

if HAVE_INTROSPECTION

# NOTE: 1 here is $API_VERSION

HudClient-1.gir: libhud-client.la

HudClient_1_gir_INCLUDES = Dee-1.0
HudClient_1_gir_CFLAGS = \
	-D HUD_CLIENT_COMPILATION \
	$(HUD_CFLAGS)
HudClient_1_gir_LIBS = $(builddir)/libhud-client.la
HudClient_1_gir_EXPORT_PACKAGES = hud-client-$(API_VERSION)
HudClient_1_gir_SCANNERFLAGS = --c-include "hud-client.h"
HudClient_1_gir_FILES = $(libhud_client_la_SOURCES)

INTROSPECTION_GIRS += HudClient-1.gir

endif

girdir = $(datadir)/gir-1.0
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(gir_DATA) $(typelib_DATA)

##########################
# Vapi
##########################

if HAVE_VALA
if HAVE_INTROSPECTION

vapidir = $(datadir)/vala/vapi
vapi_DATA = HudClient-1.vapi

HudClient-1.vapi: HudClient-1.gir
	$(VALA_API_GEN) --pkg gio-2.0 --library=HudClient-1 $<

CLEANFILES += $(vapi_DATA)

endif
endif

