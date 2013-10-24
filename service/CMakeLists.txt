
###########################
# Lib Hud Service
###########################

set(HUD_SERVICE_LIB_SOURCES
  HudService.cpp
  Query.cpp
)

qt5_add_dbus_adaptor(
  HUD_SERVICE_LIB_SOURCES
  ${HUD_SERVICE_XML}
  service/HudService.h
  hud::service::HudService
  HudAdaptor
)

qt5_add_dbus_adaptor(
  HUD_SERVICE_LIB_SOURCES
  ${HUD_QUERY_XML}
  service/Query.h
  hud::service::Query
  QueryAdaptor
)

qt5_add_dbus_adaptor(
  HUD_SERVICE_LIB_SOURCES
  ${HUD_APP_XML}
  service/Application.h
  hud::service::Application
  ApplicationAdaptor
)

add_library(hud-service
  STATIC
  ${HUD_SERVICE_LIB_SOURCES}
)

target_link_libraries(hud-service
  hud-common
)

qt5_use_modules(
	hud-service
	Core
	DBus
)

###########################
# Hud Service Executable
###########################

add_executable(hud-service-exec
  main.cpp
)

set_target_properties(hud-service-exec
  PROPERTIES
    OUTPUT_NAME "hud-service"
 )

target_link_libraries(hud-service-exec
  hud-service
)

qt5_use_modules(
	hud-service-exec
	Core
)

###########################
# Installation
###########################

install(
  TARGETS hud-service-exec
  RUNTIME DESTINATION "${CMAKE_INSTALL_FULL_LIBEXECDIR}/hud"
)
