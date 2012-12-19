check_PROGRAMS = gimp consumer

gimp_CFLAGS = $(HUD_CFLAGS) -I$(top_srcdir) -I$(top_srcdir)/libhud
gimp_LDADD = $(top_builddir)/libhud/libhud.la $(HUD_LIBS)

consumer_CFLAGS = $(HUD_CFLAGS) -I$(top_srcdir) -I$(top_srcdir)/libhud
consumer_LDADD = $(HUD_LIBS)
consumer_VALAFLAGS = --pkg gtk+-3.0
consumer_SOURCES = \
	yodawgiheardyoulikeactiongroup.h \
	yodawgiheardyoulikeactiongroup.c \
	yodawgiheardyoulikeactiongroup.vapi	\
	gvtk.vala \
	consumer.vala
