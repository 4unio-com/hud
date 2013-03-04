
EXTRA_DIST = 
bin_PROGRAMS = hud-gtk


####################
# hud-gtk
####################

hudgtkdatadir = $(datadir)/hud-gtk
hudgtkdata_DATA = hud-gtk.ui
EXTRA_DIST += hud-gtk.ui

hud_gtk_SOURCES = \
	hud-gtk.vala

hud_gtk_VALAFLAGS = \
	--pkg gtk+-3.0 \
	--pkg Dee-1.0 \
	--pkg HudClient-1 \
	--vapidir $(top_builddir)/libhud-client

hud_gtk_CFLAGS = \
	-DHUD_GTK_DATADIR=\""$(hudgtkdatadir)"\" \
	-I $(top_srcdir) \
	-I $(top_srcdir)/libhud-client \
	$(gtk_CFLAGS) \
	$(HUD_CFLAGS)

hud_gtk_LDADD = \
	$(top_builddir)/libhud-client/libhud-client.la \
	$(gtk_LIBS) \
	$(HUD_LIBS)

