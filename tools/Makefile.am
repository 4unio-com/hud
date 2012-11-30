# This will make stuff

CLEANFILES =
DISTCLEANFILES =
BUILT_SOURCES =
EXTRA_DIST =
bin_PROGRAMS =

####################
# HUD CLI
####################

hud_cli_SOURCES = \
	hud-cli.c

hud_cli_CFLAGS = \
	$(HUD_CFLAGS) \
	$(COVERAGE_CFLAGS) \
	-I $(top_srcdir) \
	-I $(top_srcdir)/libhud-client \
	-Wall -Werror

hud_cli_LDFLAGS = \
	$(HUD_LIBS) \
	$(COVERAGE_LDFLAGS) \
	-lreadline -lcurses

hud_cli_LDADD = \
	$(top_builddir)/libhud-client/libhud-client.la

if BUILD_CLI
bin_PROGRAMS += \
	hud-cli
else
EXTRA_DIST += \
	$(hud_cli_SOURCES)
endif


####################
# Extra stuff
####################

CLEANFILES += $(BUILT_SOURCES)
