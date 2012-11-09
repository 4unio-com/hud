gsettings_SCHEMAS = \
    com.canonical.indicator.appmenu.hud.gschema.xml \
    com.canonical.indicator.appmenu.hud.search.gschema.xml

@GSETTINGS_RULES@

dbus_servicesdir = $(DBUSSERVICEDIR)
dbus_services_DATA = com.canonical.hud.service

%.service: %.service.in
	sed -e "s|\@libexecdir\@|$(libexecdir)|" $< > $@

dbus_interfacesdir = $(DBUSIFACEDIR)
dbus_interfaces_DATA = \
	com.canonical.hud.query.xml \
	com.canonical.hud.xml

EXTRA_DIST = \
	$(dbus_interfaces_DATA) \
	$(dbus_services_DATA:.service=.service.in) \
	$(gsettings_SCHEMAS)

CLEANFILES = \
	$(dbus_services_DATA)
