
set(
	UNIT_TESTS_SRC
	TestApplication.cpp
	TestApplicationList.cpp
	TestHudService.cpp
	TestVoice.cpp
	TestWindow.cpp
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

add_test(
	service-unit-tests
	qdbus-simple-test-runner "${CMAKE_CURRENT_BINARY_DIR}/service-unit-tests"
)
