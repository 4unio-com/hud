


set(
	WINDOW_STACK_BRIDGE_SOURCES
	WindowStack.cpp
)

qt5_add_dbus_adaptor(
	WINDOW_STACK_BRIDGE_SOURCES
	${WINDOW_STACK_XML}
	WindowStack.h
	WindowStack
	WindowStackAdaptor
)

set_source_files_properties(
	"${DATA_DIR}/org.freedesktop.Notifications.xml"
	PROPERTIES
	INCLUDE "DBusTypes.h"
)

add_library(
	window-stack-bridge
	STATIC
	${WINDOW_STACK_BRIDGE_SOURCES}
)

qt5_use_modules(
	window-stack-bridge
	Core
	DBus
)

add_executable(
	window-stack-bridge-bin
	main.cpp
)

set_target_properties(
	window-stack-bridge-bin
	PROPERTIES
	OUTPUT_NAME window-stack-bridge
)

qt5_use_modules(
	window-stack-bridge-bin
	Core
)

target_link_libraries(
	window-stack-bridge-bin
	window-stack-bridge
)

install(
	TARGETS window-stack-bridge-bin
	RUNTIME DESTINATION "${CMAKE_INSTALL_LIBEXECDIR}"
)
