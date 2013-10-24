
set(HUD_SERVICE_INCLUDE_DIRS
${CMAKE_CURRENT_SOURCE_DIR}
${CMAKE_CURRENT_BINARY_DIR}
)

set(HUD_SERVICE_INCLUDE_DIRS
${HUD_SERVICE_INCLUDE_DIRS}
PARENT_SCOPE
)

include_directories(${HUD_SERVICE_INCLUDE_DIRS})
include_directories(${HUD_CLIENT_INCLUDE_DIRS})

if(${FULL_WARNINGS})
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
endif()

add_constant_template(GENERATED_SOURCES
create-db
create_db
"${CMAKE_CURRENT_SOURCE_DIR}/create-db.sql"
)

###########################
# Hud Dump Application
###########################

add_executable(hud-dump-application
  hud-dump-application.c
  dump-app-info.c
)

target_link_libraries(hud-dump-application
  ${GLIB2_LIBRARIES}
  ${SQLITE3_LIBRARIES}
)

###########################
# Hud Verify App Info
###########################

add_executable(hud-verify-app-info
  hud-verify-app-info.c
  load-app-info.c
  dump-app-info.c
  ${GENERATED_SOURCES}
)

target_link_libraries(hud-verify-app-info
  ${GLIB2_LIBRARIES}
  ${GIO2_LIBRARIES}
  ${SQLITE3_LIBRARIES}
)

###########################
# Installation
###########################

install(
  PROGRAMS
    hud-list-applications
  DESTINATION
    ${CMAKE_INSTALL_BINDIR}
)

install(
  TARGETS
    hud-dump-application
    hud-verify-app-info
  RUNTIME DESTINATION
    ${CMAKE_INSTALL_BINDIR}
)
