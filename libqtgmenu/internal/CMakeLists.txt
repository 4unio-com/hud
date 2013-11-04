set(
        QTGMENU_INTERNAL_SRC
        QtGMenuImporterPrivate.cpp
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
