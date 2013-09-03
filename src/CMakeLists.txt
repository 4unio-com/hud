
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

###########################
# Generated Lib Hud Service
###########################

set(HUD_SERVICE_GENERATED_HEADERS
app-iface.h
hud-iface.h
hud-query-iface.h
)

set(HUD_SERVICE_GENERATED_SOURCES
)

add_gdbus_codegen(
OUTFILES HUD_SERVICE_GENERATED_SOURCES
NAME hud-iface
PREFIX com.canonical.hud.
NAMESPACE HudIface
SERVICE_XML ${HUD_SERVICE_XML}
)

add_gdbus_codegen(
OUTFILES HUD_SERVICE_GENERATED_SOURCES
NAME hud-query-iface
PREFIX com.canonical.hud.query.
NAMESPACE HudQueryIface
SERVICE_XML ${HUD_QUERY_XML}
)

add_gdbus_codegen(
OUTFILES HUD_SERVICE_GENERATED_SOURCES
NAME app-iface
PREFIX com.canonical.hud.Application.
NAMESPACE AppIface
SERVICE_XML ${HUD_APP_XML}
)

add_constant_template(HUD_SERVICE_GENERATED_SOURCES
create-db
create_db
"${CMAKE_CURRENT_SOURCE_DIR}/create-db.sql"
)

add_library(hud-service-generated STATIC ${HUD_SERVICE_GENERATED_SOURCES})

target_link_libraries(hud-service-generated
${GLIB2_LIBRARIES}
${GOBJECT2_LIBRARIES}
)

###########################
# Lib Hud Service
###########################

set(HUD_SERVICE_LIB_HEADERS
abstract-app.h
action-muxer.h
application-list.h
application-source-context.h
application-source.h
dump-app-info.h
shared-values.h
settings.h
appmenu-registrar.h
dbusmenu-collector.h
menumodel-collector.h
appindicator-source.h
webapp-source.h
indicator-source.h
debug-source.h
source-list.h
source.h
token.h
query.h
string-list.h
voice.h
sphinx.h
julius.h
item.h
keyword-mapping.h
result.h
load-app-info.h
pronounce-dict.h
query-columns.h
usage-tracker.h
watchdog.h
)

set(HUD_SERVICE_LIB_SOURCES
action-muxer.c
application-list.c
application-source-context.c
application-source.c
dump-app-info.c
settings.c
appmenu-registrar.c
dbusmenu-collector.c
menumodel-collector.c
appindicator-source.c
webapp-source.c
indicator-source.c
debug-source.c
source-list.c
source.c
token.c
query.c
string-list.c
voice.c
sphinx.c
julius.c
item.c
keyword-mapping.c
result.c
load-app-info.c
pronounce-dict.c
usage-tracker.c
watchdog.c
)

add_glib_enumtypes_t(HUD_SERVICE_LIB_SOURCES
enum-types
enum-types.h.in
enum-types.c.in
${HUD_SERVICE_LIB_HEADERS}
)

add_library(hud-service STATIC ${HUD_SERVICE_LIB_SOURCES})

target_link_libraries(hud-service
hud-client
hud-service-generated
${GLIB2_LIBRARIES}
${GOBJECT2_LIBRARIES}
${GIO2_LIBRARIES}
${BAMF_LIBRARIES}
${DEE_LIBRARIES}
${DBUSMENU_GLIB_LIBRARIES}
${SQLITE3_LIBRARIES}
${POCKETSPHINX_LIBRARIES}
${PHABLET_LIBRARIES}
${NIH_LIBRARIES}
${NIH_DBUS_LIBRARIES}
)

###########################
# Hud Service Executable
###########################

add_executable(hud-service-exec service.c)

set_target_properties(hud-service-exec PROPERTIES OUTPUT_NAME "hud-service")

target_link_libraries(hud-service-exec hud-service)

###########################
# Hud Dump Application
###########################

add_executable(hud-dump-application hud-dump-application.c)

target_link_libraries(hud-dump-application hud-service)

###########################
# Hud Verify App Info
###########################

add_executable(hud-verify-app-info hud-verify-app-info.c)

target_link_libraries(hud-verify-app-info hud-service)

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

install(
  TARGETS hud-service-exec
  RUNTIME DESTINATION "${CMAKE_INSTALL_FULL_LIBEXECDIR}/hud"
)
