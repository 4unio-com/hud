
set(
	TEST_UTILS_SRC
	MockHudService.cpp
	main.cpp
)

add_library(
	test-utils
	SHARED
	${TEST_UTILS_SRC}
)

qt5_use_modules(
	test-utils
	Core
	Widgets
)

target_link_libraries(
	test-utils
	hud-common
)
