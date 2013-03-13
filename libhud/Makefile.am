
CLEANFILES =
DISTCLEANFILES =
BUILT_SOURCES =
EXTRA_DIST =

SUBDIRS = . tests demo

lib_LTLIBRARIES = libhud.la

ABI_VERSION = 1
API_VERSION = 1

libhud_include_basedir = $(includedir)/libhud-$(API_VERSION)/
libhud_include_base_HEADERS = \
	hud.h

libhudincludedir = $(includedir)/libhud-$(API_VERSION)/libhud
libhudinclude_HEADERS = \
	manager.h		\
	operation.h		\
	action.h		\
	action-publisher.h

libhud_la_SOURCES = \
	manager.h		\
	manager.c		\
	operation-private.h	\
	operation.h		\
	operation.c		\
	action.c		\
	action-publisher.h \
	action-publisher.c

libhud_la_CFLAGS = \
	$(HUD_CFLAGS) \
	$(COVERAGE_CFLAGS) \
	-I . \
	-I $(top_srcdir) \
	-D G_LOG_DOMAIN=\"libhud\" \
	-D HUD_COMPILATION \
	-Wall -Werror

libhud_la_LIBADD = \
	$(HUD_LIBS) \
	libhud-generated.la

libhud_la_LDFLAGS = \
	$(HUD_LDFLAGS) \
	$(COVERAGE_LDFLAGS) \
	-version-info $(ABI_VERSION):0:0 \
	-no-undefined \
	-export-symbols-regex "^[^_].*"

#################################
# Generated Code
#################################

noinst_LTLIBRARIES = libhud-generated.la

libhud_generated_la_SOURCES = \
	app-iface.h	\
	app-iface.c	\
	service-iface.h	\
	service-iface.c	\
	marshal.h \
	marshal.c

libhud_generated_la_CFLAGS = \
	$(HUD_CFLAGS)

libhud_generated_la_LIBADD = \
	$(HUD_LIBS)

libhud_generated_la_LDFLAGS = \
	$(HUD_LDFLAGS)

# Marshalling

include $(top_srcdir)/Makefile.am.marshal

glib_marshal_list = marshal.list
glib_marshal_prefix = _hud_marshal

# DBus Ifaces

service-iface.c: $(top_srcdir)/data/com.canonical.hud.xml
	$(AM_V_GEN) gdbus-codegen \
		--interface-prefix com.canonical.hud. \
		--generate-c-code service-iface \
		--c-namespace _HudServiceIface \
		$^
service-iface.h: service-iface.c

app-iface.c: $(top_srcdir)/data/com.canonical.hud.Application.xml
	$(AM_V_GEN) gdbus-codegen \
		--interface-prefix com.canonical.hud.Application. \
		--generate-c-code app-iface \
		--c-namespace _HudAppIface \
		$^
app-iface.h: app-iface.c

BUILT_SOURCES += \
	app-iface.h \
	app-iface.c \
	service-iface.h \
	service-iface.c

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

Hud-1.gir: libhud.la

Hud_1_gir_INCLUDES = Dee-1.0
Hud_1_gir_CFLAGS = \
	-D HUD_COMPILATION \
	$(HUD_CFLAGS)
Hud_1_gir_LIBS = $(builddir)/libhud.la
Hud_1_gir_EXPORT_PACKAGES = hud-$(API_VERSION)
Hud_1_gir_SCANNERFLAGS = --c-include "hud.h"
Hud_1_gir_FILES = $(libhud_la_SOURCES)

INTROSPECTION_GIRS += Hud-1.gir

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
vapi_DATA = Hud-1.vapi

Hud-1.vapi: Hud-1.gir
	$(VALA_API_GEN) --pkg gio-2.0 --library=Hud-1 $<

CLEANFILES += $(vapi_DATA)

endif
endif


#################################
# Package Config
#################################

EXTRA_DIST += \
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

CLEANFILES += \
	$(BUILD_SOURCES) \
	hud-$(API_VERSION).pc

