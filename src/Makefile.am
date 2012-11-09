CLEANFILES =
DISTCLEANFILES =
BUILT_SOURCES =
EXTRA_DIST = \
	hud-list-applications

bin_PROGRAMS = \
	hud-dump-application \
	hud-verify-app-info

bin_SCRIPTS = \
	hud-list-applications

libexec_PROGRAMS = \
	hud-service

include $(top_srcdir)/Makefile.am.marshal

####################
# HUD Service
####################

noinst_LIBRARIES = libhud-service.a

libhud_service_a_SOURCES = \
	create-db.h \
	create-db.c \
	dump-app-info.h \
	dump-app-info.c \
	shared-values.h \
	hudsettings.h		\
	hudsettings.c		\
	hudappmenuregistrar.h	\
	hudappmenuregistrar.c	\
	huddbusmenucollector.h	\
	huddbusmenucollector.c	\
	hudmenumodelcollector.h	\
	hudmenumodelcollector.c	\
	hudappindicatorsource.h	\
	hudappindicatorsource.c	\
	hudwebappsource.h \
	hudwebappsource.c \
	hudindicatorsource.h	\
	hudindicatorsource.c	\
	hudwindowsource.h	\
	hudwindowsource.c	\
	huddebugsource.h	\
	huddebugsource.c	\
	hudsourcelist.h		\
	hudsourcelist.c		\
	hudsource.h		\
	hudsource.c		\
	hudtoken.h		\
	hudtoken.c		\
	hudquery.h		\
	hudquery.c		\
	hudstringlist.h		\
	hudstringlist.c		\
	huditem.h		\
	huditem.c		\
	hudresult.h		\
	hudresult.c		\
	hud-iface.h \
	hud-iface.c \
	hud-query-iface.h \
	hud-query-iface.c \
	load-app-info.h \
	load-app-info.c \
	usage-tracker.h \
	usage-tracker.c

hud_service_SOURCES = hud-service.c

libhud_service_a_CFLAGS = \
	$(HUD_CFLAGS) \
	$(COVERAGE_CFLAGS) \
	-DDATADIR=\""$(datadir)"\" \
	-Wall -Werror
hud_service_CFLAGS = \
	$(HUD_CFLAGS) \
	$(COVERAGE_CFLAGS) \
	-DDATADIR=\""$(datadir)"\" \
	-Wall -Werror
hud_service_LDADD = \
	libhud-service.a	\
	$(HUD_LIBS)
hud_service_LDFLAGS = \
	$(COVERAGE_LDFLAGS)


create-db.c: create-db.sql
	echo "#include \"create-db.h\"" > $@
	echo -n "const gchar * create_db = \"" >> $@
	cat $< | xargs printf "%s " >> $@
	echo -n "\";" >> $@

CLEANFILES += \
	create-db.c


####################
# HUD CLI
####################

hud_cli_SOURCES = \
	shared-values.h \
	hud-cli.c \
	hud.interface.h \
	hud.interface.c

hud_cli_CFLAGS = \
	$(HUD_CFLAGS) \
	$(COVERAGE_CFLAGS) \
	-Wall -Werror

hud_cli_LDFLAGS = \
	$(HUD_LIBS) \
	$(COVERAGE_LDFLAGS) \
	-lreadline -lcurses


if BUILD_CLI
bin_PROGRAMS += \
	hud-cli
else
EXTRA_DIST += \
	$(hud_cli_SOURCES)
endif

#######################
# HUD Dump Application
#######################

hud_dump_application_SOURCES = \
	dump-app-info.h \
	dump-app-info.c \
	hud-dump-application.c

hud_dump_application_CFLAGS = \
	$(HUD_CFLAGS) \
	$(COVERAGE_CFLAGS) \
	-Wall -Werror
hud_dump_application_LDFLAGS = \
	$(COVERAGE_LDFLAGS) \
	$(HUD_LIBS)

#######################
# HUD Verify App Info
#######################

hud_verify_app_info_SOURCES = \
	create-db.h \
	create-db.c \
	load-app-info.h \
	load-app-info.c \
	hud-verify-app-info.c

hud_verify_app_info_CFLAGS = \
	$(HUD_CFLAGS) \
	$(COVERAGE_CFLAGS) \
	-Wall -Werror
hud_verify_app_info_LDFLAGS = \
	$(COVERAGE_LDFLAGS) \
	$(HUD_LIBS)

####################
# HUD DBus Interface
####################

EXTRA_DIST += \
	com.canonical.hud.xml \
	com.canonical.hud.query.xml \
	create-db.sql

BUILT_SOURCES += \
	hud-query-iface.c \
	hud-query-iface.h \
	hud-iface.c \
	hud-iface.h

hud-iface.c: $(srcdir)/com.canonical.hud.xml
	$(AM_V_GEN) gdbus-codegen \
		--interface-prefix com.canonical.hud. \
		--generate-c-code hud-iface \
		--c-namespace HudIface \
		$^
hud-iface.h: hud-iface.c

hud-query-iface.c: $(srcdir)/com.canonical.hud.query.xml
	$(AM_V_GEN) gdbus-codegen \
		--interface-prefix com.canonical.hud.query. \
		--generate-c-code hud-query-iface \
		--c-namespace HudQueryIface \
		$^
hud-query-iface.h: hud-iface.c


CLEANFILES += $(BUILT_SOURCES)

