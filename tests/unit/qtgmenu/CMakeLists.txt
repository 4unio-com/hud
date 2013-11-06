set(
    UNIT_TESTS_SRC
    TestQtGMenu.cpp
)

add_executable(
    qtgmenu-unit-tests
    ${UNIT_TESTS_SRC}
)

qt5_use_modules(
    qtgmenu-unit-tests
    Test
)

target_link_libraries(
    qtgmenu-unit-tests
    qtgmenu
    test-utils
    hud-service
    ${GIO2_LIBRARIES}
    ${GTEST_LIBRARIES}
    ${GMOCK_LIBRARIES}
    ${QTDBUSTEST_LIBRARIES}
    ${QTDBUSMOCK_LIBRARIES}
)

add_valgrind_test(
    qtgmenu-unit-tests
    qtgmenu-unit-tests
)
