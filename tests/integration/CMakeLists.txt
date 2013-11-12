
set(
	INTEGRATION_TESTS_SRC
	TestHudService.cpp
)

add_executable(
	integration-tests
	${INTEGRATION_TESTS_SRC}
)

target_link_libraries(
	integration-tests
	test-utils
	hud-common
	${GTEST_LIBRARIES}
	${GMOCK_LIBRARIES}
	${QTDBUSTEST_LIBRARIES}
	${QTDBUSMOCK_LIBRARIES}
)

add_valgrind_test(
	integration-tests
	integration-tests
)
