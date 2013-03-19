
set(HUD_INCLUDE_DIRS
${CMAKE_CURRENT_SOURCE_DIR}
${CMAKE_CURRENT_BINARY_DIR}
)

set(HUD_INCLUDE_DIRS
${HUD_INCLUDE_DIRS}
PARENT_SCOPE
)

include_directories(${HUD_INCLUDE_DIRS})

# FIXME Would rather not include this
include_directories(${HUD_SRC_DIR})

add_definitions( -DHUD_COMPILATION=1 )

###########################
# Version Information
###########################

set(API_VERSION 1)
set(ABI_VERSION 1)

#libhud_include_basedir = $(includedir)/libhud-$(API_VERSION)/

###########################
# Generated Lib Hud
###########################

set(HUD_GENERATED_HEADERS
app-iface.h
service-iface.h
marshal.h
)

set(HUD_GENERATED_SOURCES
)

add_gdbus_codegen(HUD_GENERATED_SOURCES
service-iface
com.canonical.hud.
_HudServiceIface
${HUD_SERVICE_XML}
)

add_gdbus_codegen(HUD_GENERATED_SOURCES
app-iface
com.canonical.hud.Application.
_HudAppIface
${HUD_APP_XML}
)

add_glib_marshal(HUD_GENERATED_SOURCES
marshal
_hud_marshal
marshal.h
)

add_library(hud-generated STATIC ${HUD_GENERATED_SOURCES})

target_link_libraries(hud-generated
${GLIB2_LIBRARIES}
${GOBJECT2_LIBRARIES}
)

###########################
# Lib Hud
###########################

set(HUD_HEADERS
manager.h
operation.h
action.h
action-publisher.h
${HUD_HEADERS}
)

set(HUD_SOURCES
manager.c
operation.c
action.c
action-publisher.c
)

add_library(hud STATIC ${HUD_SOURCES})

set_target_properties(hud PROPERTIES VERSION ${API_VERSION} SOVERSION ${ABI_VERSION})

target_link_libraries(hud
hud-generated
${GLIB2_LIBRARIES}
${GOBJECT2_LIBRARIES}
-Wl,--no-undefined
)

###########################
# Pkg Config
###########################

set(HUD_PC "hud-${API_VERSION}.pc")
set(apiversion "${API_VERSION}")
set(libdir "${CMAKE_INSTALL_FULL_LIBDIR}")
set(includedir "${CMAKE_INSTALL_FULL_INCLUDEDIR}")
set(VERSION "${ABI_VERSION}")
configure_file("hud.pc.in" "${CMAKE_CURRENT_BINARY_DIR}/${HUD_PC}" @ONLY)

###########################
# Installation
###########################

#install(
#  TARGETS ${HUD_LIB_BASENAME}
#  ARCHIVE DESTINATION ${LIBDIR}
#  RUNTIME DESTINATION bin
#  LIBRARY DESTINATION ${LIBDIR}
#)
