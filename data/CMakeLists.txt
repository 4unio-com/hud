
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
DESTINATION "${CMAKE_INSTALL_DATADIR}/upstart/sessions"
)

set(pkglibexecdir "${CMAKE_INSTALL_FULL_LIBEXECDIR}/hud")
configure_file("dbus-activation-hack.sh.in"
  "${CMAKE_CURRENT_BINARY_DIR}/dbus-activation-hack.sh"
  @ONLY
)

install(
PROGRAMS
"${CMAKE_CURRENT_BINARY_DIR}/dbus-activation-hack.sh"
DESTINATION
"${CMAKE_INSTALL_FULL_LIBEXECDIR}/hud"
)

###########################
# Window Stack Bridge
###########################

configure_file(
  "window-stack-bridge.conf.in"
  "window-stack-bridge.conf"
  @ONLY
)

install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/window-stack-bridge.conf"
  DESTINATION "${CMAKE_INSTALL_DATADIR}/upstart/sessions"
)

###########################
# GSettings
###########################

add_schema ("com.canonical.indicator.appmenu.hud.gschema.xml")
add_schema ("com.canonical.indicator.appmenu.hud.search.gschema.xml")

###########################
# Desktop files
###########################

install(
  FILES hud-gui.desktop
  DESTINATION "${CMAKE_INSTALL_DATADIR}/applications"
)
