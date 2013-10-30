
set(
	UNIT_TESTS_SRC
	TestApplicationList.cpp
	TestHudService.cpp
)

add_executable(
	service-unit-tests
	${UNIT_TESTS_SRC}
)

target_link_libraries(
	service-unit-tests
	test-utils
	hud-service
	${GTEST_LIBRARIES}
	${GMOCK_LIBRARIES}
	${QTDBUSTEST_LIBRARIES}
	${QTDBUSMOCK_LIBRARIES}
)

add_valgrind_test(
	service-unit-tests
	service-unit-tests
)
