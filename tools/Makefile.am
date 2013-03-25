# This will make stuff

CLEANFILES =
DISTCLEANFILES =
BUILT_SOURCES =
EXTRA_DIST =
bin_PROGRAMS =

SUBDIRS = hudkeywords

#######################
# HUD Pronounce
#######################

hud_pronounce_for_word_SOURCES = \
	hud-pronounce-for-word.c

hud_pronounce_for_word_CFLAGS = \
	$(gtk_CFLAGS) \
	$(HUD_CFLAGS) \
	$(COVERAGE_CFLAGS) \
	$(gtk_CFLAGS) \
	-I $(top_srcdir) \
	-I $(top_srcdir)/src \
	-Wall -Werror

hud_pronounce_for_word_LDFLAGS = \
	$(top_builddir)/src/libhud-service.la \
	$(COVERAGE_LDFLAGS) \
	$(HUD_LIBS)

bin_PROGRAMS += \
	hud-pronounce-for-word

####################
# HUD CLI
####################

hud_cli_SOURCES = \
	hud-cli.c

hud_cli_CFLAGS = \
	$(gtk_CFLAGS) \
	$(HUD_CFLAGS) \
	$(COVERAGE_CFLAGS) \
	$(gtk_CFLAGS) \
	-I $(top_srcdir) \
	-I $(top_srcdir)/libhud-client \
	-Wall -Werror

hud_cli_LDFLAGS = \
	$(HUD_LIBS) \
	$(COVERAGE_LDFLAGS) \
	$(gtk_LIBS)

hud_cli_LDADD = \
	$(gtk_LIBS) \
	$(top_builddir)/libhud-client/libhud-client.la

bin_PROGRAMS += \
	hud-cli

####################
# HUD CLI Param
####################

hud_cli_param_SOURCES = \
	hud-cli-param.c

hud_cli_param_CFLAGS = \
	$(gtk_CFLAGS) \
	$(HUD_CFLAGS) \
	$(COVERAGE_CFLAGS) \
	$(gtk_CFLAGS) \
	-I $(top_srcdir) \
	-I $(top_srcdir)/libhud-client \
	-Wall -Werror

hud_cli_param_LDFLAGS = \
	$(HUD_LIBS) \
	$(COVERAGE_LDFLAGS) \
	$(gtk_LIBS)

hud_cli_param_LDADD = \
	$(gtk_LIBS) \
	$(top_builddir)/libhud-client/libhud-client.la

bin_PROGRAMS += \
	hud-cli-param

####################
# HUD CLI Appstack
####################

hud_cli_appstack_SOURCES = \
	hud-cli-appstack.c

hud_cli_appstack_CFLAGS = \
	$(gtk_CFLAGS) \
	$(HUD_CFLAGS) \
	$(COVERAGE_CFLAGS) \
	$(gtk_CFLAGS) \
	-I $(top_srcdir) \
	-I $(top_srcdir)/libhud-client \
	-Wall -Werror

hud_cli_appstack_LDFLAGS = \
	$(HUD_LIBS) \
	$(COVERAGE_LDFLAGS) \
	$(gtk_LIBS)

hud_cli_appstack_LDADD = \
	$(gtk_LIBS) \
	$(top_builddir)/libhud-client/libhud-client.la

bin_PROGRAMS += \
	hud-cli-appstack

####################
# HUD CLI Toolbar
####################

hud_cli_toolbar_SOURCES = \
	hud-cli-toolbar.c

hud_cli_toolbar_CFLAGS = \
	$(HUD_CFLAGS) \
	-I $(top_srcdir) \
	-I $(top_srcdir)/libhud-client \
	-Wall -Werror

hud_cli_toolbar_LDFLAGS = \
	$(HUD_LIBS)

hud_cli_toolbar_LDADD = \
	$(top_builddir)/libhud-client/libhud-client.la

bin_PROGRAMS += \
	hud-cli-toolbar



####################
# Extra stuff
####################

CLEANFILES += $(BUILT_SOURCES)
