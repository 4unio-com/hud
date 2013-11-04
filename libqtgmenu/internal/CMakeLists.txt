set(
        QTGMENU_INTERNAL_SRC
        QtGMenuDBus.cpp
)

set(
        QTGMENU_XML
        "${CMAKE_SOURCE_DIR}/data/com.canonical.qtgmenu.xml"
)

qt5_generate_dbus_interface(
        QtGMenuDBus.h
        ${QTGMENU_XML}
        OPTIONS -M -S
)

qt5_add_dbus_interface(
        QTGMENU_INTERNAL_SRC
        ${QTGMENU_XML}
        QtGMenuDBusInterface
)

qt5_add_dbus_adaptor(
        QTGMENU_INTERNAL_SRC
        ${QTGMENU_XML}
        QtGMenuDBus.h
        QtGMenuDBus
        QtGMenuDBusAdaptor
)

add_library(
        qtgmenu-internal
        ${QTGMENU_INTERNAL_SRC}
)

qt5_use_modules(
        qtgmenu-internal
        Core
        DBus
)

target_link_libraries(
        qtgmenu-internal
        ${GIO2_LIBRARIES}
)
