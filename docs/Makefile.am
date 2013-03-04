SUBDIRS = \
	libhud-client \
	man

EXTRA_DIST = \
	HUD Architecture.svg \
	$(CHARTS)

CHARTS = \
	libhud-communication.msc

charts: $(CHARTS:.msc=.png)

CLEANFILES = \
	$(CHARTS:.msc=.png)

%.png: %.msc
	mscgen -T png -i $< -o $@
