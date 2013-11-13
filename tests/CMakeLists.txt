set(CMAKE_AUTOMOC OFF)
include(FindGMock)
set(CMAKE_AUTOMOC ON)

find_package("Valgrind" REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${GMOCK_INCLUDE_DIRS})
include_directories(${GTEST_INCLUDE_DIRS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

add_definitions(-DHUD_SERVICE_BINARY="${CMAKE_BINARY_DIR}/service/hud-service")

add_definitions(-DTEST_LOCALEDIR="${CMAKE_CURRENT_BINARY_DIR}/data")
set(TEST_DATADIR "${CMAKE_CURRENT_SOURCE_DIR}/data")
add_definitions(-DTEST_DATADIR="${TEST_DATADIR}")

add_definitions(-DDATA_DIR="${DATA_DIR}")

#######################
# Test Fixtures
#######################

message("dat adir = ${TEST_DATADIR}")

add_definitions(-DJSON_SIMPLE="${TEST_DATADIR}/test-menu-input-simple.json")
add_definitions(-DJSON_SHORTCUTS="${TEST_DATADIR}/test-menu-input-shortcuts.json")
add_definitions(-DJSON_SOURCE_ONE="${TEST_DATADIR}/test-app-indicator-source-one.json")
add_definitions(-DJSON_SOURCE_TWO="${TEST_DATADIR}/test-app-indicator-source-two.json")
add_definitions(-DJSON_SOURCE_SESSION="${TEST_DATADIR}/test-indicator-source-session.json")
add_definitions(-DJSON_SOURCE_DATETIME="${TEST_DATADIR}/test-indicator-source-datetime.json")
add_definitions(-DJSON_SOURCE_SOUND="${TEST_DATADIR}/test-indicator-source-sound.json")
add_definitions(-DJSON_SOURCE="${TEST_DATADIR}/test-source.json")

#######################
# Dbus JSON Loader
#######################

pkg_check_modules(DBUSMENU_JSONLOADER REQUIRED dbusmenu-jsonloader-0.4)
include_directories(${DBUSMENU_JSONLOADER_INCLUDE_DIRS})

add_definitions(-DDBUSMENU_JSON_LOADER="${CMAKE_CURRENT_BINARY_DIR}/dbusmenu-json-loader")

add_executable(dbusmenu-json-loader dbusmenu-json-loader.c)
target_link_libraries(dbusmenu-json-loader
${DBUSMENU_JSONLOADER_LIBRARIES}
)

#add_subdirectory(data)
add_subdirectory(integration)
add_subdirectory(unit)
add_subdirectory(testutils)
