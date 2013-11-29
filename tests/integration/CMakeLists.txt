
set(
	INTEGRATION_TESTS_SRC
	TestHud.cpp
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
	qtgmenu
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
