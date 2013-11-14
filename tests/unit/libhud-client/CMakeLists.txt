
set(
	UNIT_TESTS_SRC
	TestQuery.cpp
)

add_executable(
	libhud-client-unit-tests
	${UNIT_TESTS_SRC}
)

target_link_libraries(
	libhud-client-unit-tests
	test-utils
	hud-client
	${GTEST_LIBRARIES}
	${GMOCK_LIBRARIES}
	${QTDBUSTEST_LIBRARIES}
	${QTDBUSMOCK_LIBRARIES}
)

qt5_use_modules(
	libhud-client-unit-tests
	Test
)

add_test(
	libhud-client-unit-tests
	qdbus-simple-test-runner "${CMAKE_CURRENT_BINARY_DIR}/libhud-client-unit-tests"
)

