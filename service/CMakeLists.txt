
add_definitions(
	-pedantic
	-Wall
	-Wextra
)

###########################
# Lib Hud Service
###########################

set(HUD_SERVICE_LIB_SOURCES
  Application.cpp
  ApplicationImpl.cpp
  ApplicationList.cpp
  ApplicationListImpl.cpp
  Collector.cpp
  DBusMenuCollector.cpp
  Factory.cpp
  GMenuCollector.cpp
  HudService.cpp
  Item.cpp
  ItemStore.cpp
  Query.cpp
  QueryImpl.cpp
  Result.cpp
  Voice.cpp
  VoiceImpl.cpp
  Window.cpp
  WindowImpl.cpp
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
  service/QueryImpl.h
  hud::service::QueryImpl
  QueryAdaptor
)

qt5_add_dbus_adaptor(
  HUD_SERVICE_LIB_SOURCES
  ${HUD_APP_XML}
  service/ApplicationImpl.h
  hud::service::ApplicationImpl
  ApplicationAdaptor
)

qt5_add_dbus_interface(
  HUD_SERVICE_LIB_SOURCES
  ${APPMENU_REGISTRAR_XML}
  AppmenuRegistrarInterface
)

add_library(hud-service
  STATIC
  ${HUD_SERVICE_LIB_SOURCES}
)

target_link_libraries(hud-service
  hud-common
  qtgmenu
  ${DBUSMENU_LIBRARIES}
  ${GOBJECT2_LIBRARIES}
  ${DEE_LIBRARIES}
  ${COLUMBUS_LIBRARIES}
)

qt5_use_modules(
	hud-service
	Core
	DBus
	Widgets
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
