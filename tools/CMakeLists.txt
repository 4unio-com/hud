
add_subdirectory(hudkeywords)

include_directories(${HUD_CLIENT_INCLUDE_DIRS})
include_directories(${HUD_SERVICE_INCLUDE_DIRS})

if(${READLINE_FOUND} AND ${READLINE_HISTORY_FOUND} AND ${CURSES_FOUND} AND ${PANGO_FOUND})

include_directories(${PANGO_INCLUDE_DIRS})

###########################
# Hud CLI
###########################

add_executable(hud-cli hud-cli.c)

target_link_libraries(hud-cli
hud-service
${CURSES_LIBRARIES}
${READLINE_LIBRARIES}
${PANGO_LIBRARIES}
)

install(
  TARGETS hud-cli
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)

###########################
# Hud CLI Param
###########################

add_executable(hud-cli-param hud-cli-param.c)

target_link_libraries(hud-cli-param
hud-service
${PANGO_LIBRARIES}
)

install(
  TARGETS hud-cli-param
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)

endif()

###########################
# Hud CLI Appstack
###########################

add_executable(hud-cli-appstack hud-cli-appstack.c)

target_link_libraries(hud-cli-appstack hud-service)

install(
  TARGETS hud-cli-appstack
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)

###########################
# Hud CLI Toolbar
###########################

add_executable(hud-cli-toolbar hud-cli-toolbar.c)

target_link_libraries(hud-cli-toolbar hud-service)

install(
  TARGETS hud-cli-toolbar
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)

###########################
# Hud Pronounce For Word
###########################

add_executable(hud-pronounce-for-word hud-pronounce-for-word.c)

target_link_libraries(hud-pronounce-for-word hud-service)

install(
  TARGETS hud-pronounce-for-word
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)