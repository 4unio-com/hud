
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
# Upstart Job
###########################

set(HUD_UPSTART
"${CMAKE_CURRENT_BINARY_DIR}/hud.conf"
)

set(pkglibexecdir "${CMAKE_INSTALL_FULL_LIBEXECDIR}/hud")
configure_file("hud.conf.in"
  "${HUD_UPSTART}"
  @ONLY
)

install(
FILES
"${HUD_UPSTART}"
DESTINATION "/usr/share/upstart/sessions"
)

set(pkglibexecdir "${CMAKE_INSTALL_FULL_LIBEXECDIR}/hud")
configure_file("dbus-activation-hack.sh.in"
  "dbus-activation-hack.sh"
  @ONLY
)

install(
PROGRAMS
"dbus-activation-hack.sh"
DESTINATION
"${CMAKE_INSTALL_FULL_LIBEXECDIR}/hud"
)

###########################
# GSettings
###########################

add_schema ("com.canonical.indicator.appmenu.hud.gschema.xml")
add_schema ("com.canonical.indicator.appmenu.hud.search.gschema.xml")
