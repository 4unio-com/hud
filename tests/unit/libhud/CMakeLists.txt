
set(
	UNIT_TESTS_SRC
	TestManager.cpp
	TestActionDescription.cpp
	TestActionPublisher.cpp
)

add_executable(
	libhud-unit-tests
	${UNIT_TESTS_SRC}
)

target_link_libraries(
	libhud-unit-tests
	test-utils
	hud
	${GTEST_LIBRARIES}
	${GMOCK_LIBRARIES}
	${QTDBUSTEST_LIBRARIES}
	${QTDBUSMOCK_LIBRARIES}
)

qt5_use_modules(
	libhud-unit-tests
	Test
)

add_hud_test(
	libhud-unit-tests
)

