set(
	QTGMENU_SRC
        QtGMenu.cpp
)

set(
        QTGMENU_XML
        "${CMAKE_SOURCE_DIR}/data/com.canonical.qtgmenu.xml"
)

qt5_generate_dbus_interface(
        QtGMenu.h
        ${QTGMENU_XML}
        OPTIONS -M -S
)

qt5_add_dbus_interface(
        QTGMENU_SRC
        ${QTGMENU_XML}
        QtGMenuInterface
)

qt5_add_dbus_adaptor(
        QTGMENU_SRC
        ${QTGMENU_XML}
        QtGMenu.h
        QtGMenu
        QtGMenuAdaptor
)

add_library(
        qtgmenu
	${QTGMENU_SRC}
)

qt5_use_modules(
        qtgmenu
        Core
        DBus
)

target_link_libraries(
        qtgmenu
        ${GIO2_LIBRARIES}
)
