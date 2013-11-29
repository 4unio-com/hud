
add_definitions(
	-pedantic
	-Wall
	-Wextra
)

set(
	INTEGRATION_TESTS_SRC
	TestHud.cpp
)

qt5_add_dbus_interface(
	INTEGRATION_TESTS_SRC
	${HUD_TEST_XML}
	HudTestInterface
)

add_executable(
	integration-tests
	${INTEGRATION_TESTS_SRC}
)

target_link_libraries(
	integration-tests
	test-utils
	hud-common
	hud-client
	${GTEST_LIBRARIES}
	${GMOCK_LIBRARIES}
	${QTDBUSTEST_LIBRARIES}
	${QTDBUSMOCK_LIBRARIES}
)

qt5_use_modules(integration-tests
	Test
)

add_hud_test(
	integration-tests
)
