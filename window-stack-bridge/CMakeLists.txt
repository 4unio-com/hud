
pkg_search_module(QTBAMF libqtbamf)
if(${QTBAMF_FOUND})
	include_directories(${QTBAMF_INCLUDE_DIRS})
	add_definitions( -DENABLE_BAMF=1 )
	set(ENABLE_BAMF true)
else()
	if(${ENABLE_PLATFORM_API})
		include_directories(${QTBAMF_INCLUDE_DIRS})
		add_definitions( -DENABLE_PLATFORM_API=1 )
		#set(PHABLET_LIBRARIES "-lubuntu_application_api")
	endif()
endif()

if(${ENABLE_BAMF} OR ${ENABLE_PLATFORM_API})

set(
	WINDOW_STACK_BRIDGE_SOURCES
	AbstractWindowStack.cpp
)

if(${ENABLE_BAMF})
	list(APPEND WINDOW_STACK_BRIDGE_SOURCES BamfWindowStack.cpp)
endif()

if(${ENABLE_PLATFORM_API})
	#FIXME Add platform API implementation
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
)

install(
	TARGETS window-stack-bridge-bin
	RUNTIME DESTINATION "${CMAKE_INSTALL_LIBEXECDIR}"
)

endif()