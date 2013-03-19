
set(HUD_CLIENT_INCLUDE_DIRS
${CMAKE_CURRENT_SOURCE_DIR}
${CMAKE_CURRENT_BINARY_DIR}
)

set(HUD_CLIENT_INCLUDE_DIRS
${HUD_CLIENT_INCLUDE_DIRS}
PARENT_SCOPE
)

include_directories(${HUD_CLIENT_INCLUDE_DIRS})

# FIXME would rather not include this dir
include_directories(${HUD_SRC_DIR})

add_definitions( -DHUD_CLIENT_COMPILATION=1 )

###########################
# Version Information
###########################

set(API_VERSION 1)
set(ABI_VERSION 1)

###########################
# Generated Hud Client
###########################

set(HUD_CLIENT_GENERATED_HEADERS
service-iface.h
query-iface.h
)

set(HUD_CLIENT_GENERATED_SOURCES
)

add_gdbus_codegen(HUD_CLIENT_GENERATED_SOURCES
service-iface
com.canonical.hud.
_HudService
${HUD_SERVICE_XML}
)

add_gdbus_codegen(HUD_CLIENT_GENERATED_SOURCES
query-iface
com.canonical.hud.query.
_HudQuery
${HUD_QUERY_XML}
)

add_library(hud-client-generated STATIC ${HUD_CLIENT_GENERATED_SOURCES})

target_link_libraries(hud-client-generated
${GLIB2_LIBRARIES}
${GOBJECT2_LIBRARIES}
${GIO2_LIBRARIES}
${DEE_LIBRARIES}
)

###########################
# Hud Client
###########################

set(HUD_CLIENT_MAIN_HEADERS
hud-client.h
)

set(HUD_CLIENT_HEADERS
connection.h
param.h
query.h
)

set(HUD_CLIENT_SOURCES
connection.c
param.c
query.c
)

add_glib_enumtypes_t(HUD_CLIENT_SOURCES
enum-types
enum-types.h.in
enum-types.c.in
${HUD_CLIENT_HEADERS}
)

add_library(hud-client SHARED ${HUD_CLIENT_SOURCES})

set_target_properties(hud-client PROPERTIES
  VERSION ${API_VERSION}.0.0
  SOVERSION ${ABI_VERSION}
  OUTPUT_NAME "hud-client"
)

target_link_libraries(hud-client
hud-client-generated
${GLIB2_LIBRARIES}
${GOBJECT2_LIBRARIES}
-Wl,--no-undefined
)

###########################
# Pkg Config
###########################

set(HUD_CLIENT_PC "hud-client-${API_VERSION}.pc")
set(apiversion "${API_VERSION}")
set(libdir "${CMAKE_INSTALL_FULL_LIBDIR}")
set(includedir "${CMAKE_INSTALL_FULL_INCLUDEDIR}")
set(VERSION "${ABI_VERSION}")
configure_file("hud-client.pc.in" "${CMAKE_CURRENT_BINARY_DIR}/${HUD_CLIENT_PC}" @ONLY)

###########################
# Installation
###########################

install(
  FILES ${HUD_CLIENT_MAIN_HEADERS}
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/libhud-client-${API_VERSION}"
)

install(
  FILES ${HUD_CLIENT_HEADERS}
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/libhud-client-${API_VERSION}/libhud-client"
)

install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/${HUD_CLIENT_PC}"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
)

install(
  TARGETS hud-client
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
