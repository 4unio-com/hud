noinst_PROGRAMS = gimp consumer

gimp_CFLAGS = $(HUD_CFLAGS)
gimp_LDADD = ../libhud.la $(HUD_LIBS)

consumer_CFLAGS = $(HUD_CFLAGS)
consumer_LDADD = $(HUD_LIBS)
consumer_VALAFLAGS = --pkg gtk+-3.0
consumer_SOURCES = \
	yodawgiheardyoulikeactiongroup.c		\
	yodawgiheardyoulikeactiongroup.vapi		\
	gvtk.vala					\
	consumer.vala
