set(
    QTGMENU_INTERNAL_SRC
    QtGMenuImporterPrivate.cpp
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

add_library(
    qtgmenu-internal
    ${QTGMENU_INTERNAL_SRC}
)

qt5_use_modules(
    qtgmenu-internal
    Core
    DBus
    Widgets
)
