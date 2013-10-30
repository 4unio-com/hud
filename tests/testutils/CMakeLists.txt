
set(
	TEST_UTILS_SRC
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
