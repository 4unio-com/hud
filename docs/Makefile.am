SUBDIRS = \
	libhud-client \
	man

EXTRA_DIST = \
	HUD Architecture.svg \
	libhud-communication.msc

%.png: %.msc
	mscgen -T png -i $< -o $@
