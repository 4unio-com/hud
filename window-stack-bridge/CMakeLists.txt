
if(${ENABLE_BAMF})
	add_definitions( -DENABLE_BAMF=1 )
elseif(${ENABLE_PLATFORM_API})
	add_definitions( -DENABLE_PLATFORM_API=1 )
	set(PLATFORM_API_LIBRARIES "-lubuntu_application_api")
endif()

if(${ENABLE_BAMF} OR ${ENABLE_PLATFORM_API})

set(
	WINDOW_STACK_BRIDGE_SOURCES
	AbstractWindowStack.cpp
)

if(${ENABLE_BAMF})
	list(APPEND WINDOW_STACK_BRIDGE_SOURCES BamfWindowStack.cpp)
	
	qt5_add_dbus_interface(
		WINDOW_STACK_BRIDGE_SOURCES
		${BAMF_XML}
		BamfInterface
	)
	
	qt5_add_dbus_interface(
		WINDOW_STACK_BRIDGE_SOURCES
		${BAMF_VIEW_XML}
		BamfViewInterface
	)
endif()

if(${ENABLE_PLATFORM_API})
	list(APPEND WINDOW_STACK_BRIDGE_SOURCES PlatformApiWindowStack.cpp)
endif()

qt5_add_dbus_adaptor(
	WINDOW_STACK_BRIDGE_SOURCES
	${WINDOW_STACK_XML}
	AbstractWindowStack.h
	AbstractWindowStack
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
	hud-common
	${PLATFORM_API_LIBRARIES}
)

install(
	TARGETS window-stack-bridge-bin
	RUNTIME DESTINATION "${CMAKE_INSTALL_FULL_LIBEXECDIR}/hud"
)

endif()