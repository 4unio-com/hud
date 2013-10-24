
###########################
# Hud Common
###########################

set(HUD_COMMON_SOURCES
  Action.cpp
  ActionGroup.cpp
  Description.cpp
  DBusTypes.cpp
  MenuModel.cpp
  NameObject.cpp
  Suggestion.cpp
  WindowInfo.cpp
)

set_source_files_properties(
  ${WINDOW_STACK_XML}
  PROPERTIES
  INCLUDE "common/WindowInfo.h"
)

qt5_add_dbus_interface(
  HUD_COMMON_SOURCES
  ${WINDOW_STACK_XML}
  WindowStackInterface
)

add_library(hud-common
  STATIC
  ${HUD_COMMON_SOURCES}
)

qt5_use_modules(
	hud-common
	Core
	DBus
)

