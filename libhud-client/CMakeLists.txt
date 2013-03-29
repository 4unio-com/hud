
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

add_gdbus_codegen(
OUTFILES HUD_CLIENT_GENERATED_SOURCES
NAME service-iface
PREFIX com.canonical.hud.
NAMESPACE _HudService
SERVICE_XML ${HUD_SERVICE_XML}
)

add_gdbus_codegen(
OUTFILES HUD_CLIENT_GENERATED_SOURCES
NAME query-iface
PREFIX com.canonical.hud.query.
NAMESPACE _HudQuery
SERVICE_XML ${HUD_QUERY_XML}
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

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")

set(HUD_CLIENT_MAIN_HEADERS
hud-client.h
)

set(HUD_CLIENT_HEADERS
connection.h
param.h
query.h
${CMAKE_SOURCE_DIR}/src/query-columns.h
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
list(APPEND HUD_CLIENT_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/enum-types.h)

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
# GObject Introspection
###########################

if(${INTROSPECTION_FOUND})
  include(UseGObjectIntrospection)
  set(INTROSPECTION_GIRS)

  set(_introspection_files ${HUD_CLIENT_SOURCES} ${HUD_CLIENT_HEADERS})
  set(HudClient_1_gir "hud-client")
  set(HudClient_1_gir_INCLUDES GObject-2.0 Dee-1.0)
  
  gir_get_cflags(_cflags)
  set(HudClient_1_gir_CFLAGS ${_cflags})
  
  set(HudClient_1_gir_LIBS hud-client)
  
  list_make_absolute(_abs_introspection_files _introspection_files "${CMAKE_CURRENT_SOURCE_DIR}/")
  set(HudClient_1_gir_FILES ${_abs_introspection_files})
  set(HudClient_1_gir_SCANNERFLAGS --c-include "hud-client.h")
  set(HudClient_1_gir_EXPORT_PACKAGES "hud-client-${API_VERSION}")

  list(APPEND INTROSPECTION_GIRS HudClient-1.gir)
  gir_add_introspections(INTROSPECTION_GIRS)
else()
  message("Not performing GIR as required tools not found")
endif()

###########################
# libhud-client Vala
###########################

if(${VALA_FOUND})
if(${INTROSPECTION_FOUND})

vapi_gen(hud-client-vapi
  INPUT
  	HudClient-1.gir
  PACKAGES
    gio-2.0
  LIBRARY
    HudClient-1
)

install(
  FILES
    ${hud-client-vapi_OUTPUT}
  DESTINATION
    "${CMAKE_INSTALL_DATADIR}/vala/vapi"
)

endif()
endif()


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
