
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
	operation.h		\
	action.h		\
	action-publisher.h

libhud_la_SOURCES = \
	app-iface.h	\
	app-iface.c	\
	service-iface.h	\
	service-iface.c	\
	manager.h		\
	manager.c		\
	marshal.h \
	marshal.c \
	operation-private.h	\
	operation.c		\
	action.c		\
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
	$(HUD_LIBS)

libhud_la_LDFLAGS = \
	$(HUD_LDFLAGS) \
	$(COVERAGE_LDFLAGS) \
	-version-info $(ABI_VERSION):0:0 \
	-no-undefined \
	-export-symbols-regex "^[^_].*"

include $(top_srcdir)/Makefile.am.marshal

glib_marshal_list = marshal.list
glib_marshal_prefix = _hud_marshal

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

