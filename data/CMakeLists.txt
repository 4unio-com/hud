
###########################
# Dbus Interfaces
###########################

install(
FILES
com.canonical.hud.query.xml
com.canonical.hud.xml
DESTINATION ${DBUSIFACEDIR}
)

###########################
# Dbus Services
###########################

set(HUD_SERVICE
"${CMAKE_CURRENT_BINARY_DIR}/com.canonical.hud.service"
)

set(pkglibexecdir "${CMAKE_INSTALL_FULL_LIBEXECDIR}/hud")
configure_file("com.canonical.hud.service.in"
  "${HUD_SERVICE}"
  @ONLY
)

install(
FILES
"${HUD_SERVICE}"
DESTINATION ${DBUSSERVICEDIR}
)

###########################
# GSettings
###########################

add_schema ("com.canonical.indicator.appmenu.hud.gschema.xml")
add_schema ("com.canonical.indicator.appmenu.hud.search.gschema.xml")
