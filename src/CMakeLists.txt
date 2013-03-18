
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

include_directories(${HUD_CLIENT_INCLUDE_DIRS})
#include_directories(${HUD_INCLUDE_DIRS})
#message(STATUS "SRC INCLUDING: ${HUD_CLIENT_INCLUDE_DIRS}")
#message(STATUS "SRC INCLUDING: ${HUD_INCLUDE_DIRS}")

###########################
# Generated Lib Hud Service
###########################

#create-db.h
set(HUD_SERVICE_GENERATED_HEADERS
app-iface.h
hud-iface.h
hud-query-iface.h
)

set(HUD_SERVICE_GENERATED_SOURCES
)

add_gdbus_codegen(HUD_SERVICE_GENERATED_SOURCES
hud-iface
com.canonical.hud.
HudIface
${HUD_SERVICE_XML}
)

add_gdbus_codegen(HUD_SERVICE_GENERATED_SOURCES
hud-query-iface
com.canonical.hud.query.
HudQueryIface
${HUD_QUERY_XML}
)

add_gdbus_codegen(HUD_SERVICE_GENERATED_SOURCES
app-iface
com.canonical.hud.Application.
AppIface
${HUD_APP_XML}
)

add_constant_template(HUD_SERVICE_GENERATED_SOURCES
create-db
create_db
create-db.sql
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
application-source.h
dump-app-info.h
shared-values.h
hudsettings.h
hudappmenuregistrar.h
huddbusmenucollector.h
hudmenumodelcollector.h
hudappindicatorsource.h
hudwebappsource.h
hudindicatorsource.h
huddebugsource.h
hudsourcelist.h
hudsource.h
hudtoken.h
hudquery.h
hudstringlist.h
hudvoice.h
hudsphinx.h
hudjulius.h
huditem.h
hudkeywordmapping.h
hudresult.h
load-app-info.h
pronounce-dict.h
usage-tracker.h
)

set(HUD_SERVICE_LIB_SOURCES
action-muxer.c
application-list.c
application-source.c
dump-app-info.c
hudsettings.c
hudappmenuregistrar.c
huddbusmenucollector.c
hudmenumodelcollector.c
hudappindicatorsource.c
hudwebappsource.c
hudindicatorsource.c
huddebugsource.c
hudsourcelist.c
hudsource.c
hudtoken.c
hudquery.c
hudstringlist.c
hudvoice.c
hudsphinx.c
hudjulius.c
huditem.c
hudkeywordmapping.c
hudresult.c
load-app-info.c
pronounce-dict.c
usage-tracker.c
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
)

###########################
# Hud Service Executable
###########################

add_executable(hud-service-exec hud-service.c)

set_target_properties(hud-service-exec PROPERTIES OUTPUT_NAME "hud-service")

target_link_libraries(hud-service-exec hud-service)

###########################
# Installation
###########################

#install(
#  TARGETS ${HUD_LIB_BASENAME}
#  ARCHIVE DESTINATION ${LIBDIR}
#  RUNTIME DESTINATION bin
#  LIBRARY DESTINATION ${LIBDIR}
#)
