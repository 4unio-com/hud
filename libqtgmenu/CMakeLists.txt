add_subdirectory(internal)

set(
    QTGMENU_SRC
    QtGMenuConverter.cpp
    QtGMenuExporter.cpp
    QtGMenuImporter.cpp
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
    qtgmenu-internal
    ${GIO2_LIBRARIES}
)
