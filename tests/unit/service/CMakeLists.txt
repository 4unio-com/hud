
set(
	UNIT_TESTS_SRC
	TestApplication.cpp
	TestApplicationList.cpp
	TestHudService.cpp
	TestItemStore.cpp
	TestQuery.cpp
	TestVoice.cpp
	TestWindow.cpp
)

add_executable(
	service-unit-tests
	${UNIT_TESTS_SRC}
)

qt5_use_modules(
	service-unit-tests
	Test
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

add_hud_test(
	service-unit-tests
)
