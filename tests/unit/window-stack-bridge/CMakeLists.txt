
if(${ENABLE_BAMF} OR ${ENABLE_PLATFORM_API})

if(${ENABLE_BAMF})
	list(APPEND UNIT_TESTS_SRC TestBamfWindowStack.cpp)
endif()

if(${ENABLE_PLATFORM_API})
	list(APPEND UNIT_TESTS_SRC TestPlatformApiWindowStack.cpp)
endif()

add_executable(
	window-stack-bridge-unit-tests
	${UNIT_TESTS_SRC}
)

target_link_libraries(
	window-stack-bridge-unit-tests
	test-utils
	window-stack-bridge
	${GTEST_LIBRARIES}
	${GMOCK_LIBRARIES}
	${QTDBUSTEST_LIBRARIES}
	${QTDBUSMOCK_LIBRARIES}
)

qt5_use_modules(
	window-stack-bridge-unit-tests
	Test
)

add_valgrind_test(
	window-stack-bridge-unit-tests
	window-stack-bridge-unit-tests
)

endif()