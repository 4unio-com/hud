add_executable(
    qtgmenu-test-app
    QtGMenuTestApp.cpp
)

qt5_use_modules(
    qtgmenu-test-app
    Core
)

target_link_libraries(
    qtgmenu-test-app
    qtgmenu
)
