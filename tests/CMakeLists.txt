include_directories(${HUD_SERVICE_INCLUDE_DIRS})
include_directories(${HUD_CLIENT_INCLUDE_DIRS})
include_directories(${HUD_INCLUDE_DIRS})

if(${FULL_WARNINGS})
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
endif()

#######################
# Dbus JSON Loader
#######################

add_definitions(-DDBUSMENU_JSON_LOADER="${CMAKE_CURRENT_BINARY_DIR}/dbusmenu-json-loader")

add_executable(dbusmenu-json-loader dbusmenu-json-loader.c)
target_link_libraries(dbusmenu-json-loader
${DBUSMENU_JSONLOADER_LIBRARIES}
)

#######################
# Menu input model deep
#######################

add_definitions(-DMODEL_DEEP="${CMAKE_CURRENT_BINARY_DIR}/test-menu-input-model-deep")

add_executable(test-menu-input-model-deep test-menu-input-model-deep.c)
target_link_libraries(test-menu-input-model-deep
  ${GOBJECT2_LIBRARIES}
  ${GIO2_LIBRARIES}
)

#########################
# Menu input model simple
#########################

add_definitions(-DMODEL_SIMPLE="${CMAKE_CURRENT_BINARY_DIR}/test-menu-input-model-simple")

add_executable(test-menu-input-model-simple test-menu-input-model-simple.c)
target_link_libraries(test-menu-input-model-simple
  ${GOBJECT2_LIBRARIES}
  ${GIO2_LIBRARIES}
)

############################
# Menu input model shortcuts
############################

add_definitions(-DMODEL_SHORTCUTS="${CMAKE_CURRENT_BINARY_DIR}/test-menu-input-model-shortcuts")

add_executable(test-menu-input-model-shortcuts test-menu-input-model-shortcuts.c)
target_link_libraries(test-menu-input-model-shortcuts
  ${GOBJECT2_LIBRARIES}
  ${GIO2_LIBRARIES}
)

####################################
# Menu input model toolbar undo item
####################################

add_definitions(-DMODEL_TOOLBAR_UNDO="${CMAKE_CURRENT_BINARY_DIR}/test-menu-input-model-toolbar-undo")

add_executable(test-menu-input-model-toolbar-undo test-menu-input-model-toolbar-undo.c)
target_link_libraries(test-menu-input-model-toolbar-undo
  ${GOBJECT2_LIBRARIES}
  ${GIO2_LIBRARIES}
)

#######################################
# Menu input model toolbar unknown item
#######################################

add_definitions(-DMODEL_TOOLBAR_UNKNOWN="${CMAKE_CURRENT_BINARY_DIR}/test-menu-input-model-toolbar-unknown")

add_executable(test-menu-input-model-toolbar-unknown test-menu-input-model-toolbar-unknown.c)
target_link_libraries(test-menu-input-model-toolbar-unknown
  ${GOBJECT2_LIBRARIES}
  ${GIO2_LIBRARIES}
)

####################################
# Menu input model toolbar all items
####################################

add_definitions(-DMODEL_TOOLBAR_ALL="${CMAKE_CURRENT_BINARY_DIR}/test-menu-input-model-toolbar-all")

add_executable(test-menu-input-model-toolbar-all test-menu-input-model-toolbar-all.c)
target_link_libraries(test-menu-input-model-toolbar-all
  ${GOBJECT2_LIBRARIES}
  ${GIO2_LIBRARIES}
)

########################################
# Menu input model toolbar dynamic items
########################################

add_definitions(-DMODEL_TOOLBAR_DYNAMIC="${CMAKE_CURRENT_BINARY_DIR}/test-menu-input-model-toolbar-dynamic")

add_executable(test-menu-input-model-toolbar-dynamic test-menu-input-model-toolbar-dynamic.c)
target_link_libraries(test-menu-input-model-toolbar-dynamic
  ${GOBJECT2_LIBRARIES}
  ${GIO2_LIBRARIES}
)


#######################
# TEST MACRO
#######################

macro(hudtest tname tfilename rfilename)
  add_executable(${tname} ${tfilename})
  target_link_libraries(${tname}
    hud-service
    hud
    hud-test
  )
  add_test(${tname}
    "${CMAKE_CURRENT_SOURCE_DIR}/test-runner.sh"
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}/${tname}"
    "${CMAKE_CURRENT_BINARY_DIR}/${rfilename}"
    "${ENABLE_VOICE_TESTS}"
  )
endmacro()

#######################
# HUD Test Common Code
#######################

set(HUD_TEST_HEADERS
app-list-dummy.h
word-list.h
hudrandomsource.h
hudmanualsource.h
hudtestutils.h
)

set(HUD_TEST_SOURCES
app-list-dummy.c
hudrandomsource.c
hudmanualsource.c
hudtestutils.c
)

add_library(hud-test STATIC ${HUD_TEST_SOURCES})

target_link_libraries(hud-test
hud-service
${DBUSTEST_LIBRARIES}
)

#######################
# Test Fixtures
#######################

add_definitions(-DKEYWORD_MAPPING="${CMAKE_CURRENT_SOURCE_DIR}/keyword-mapping")

add_definitions(-DTEST_VOICE_SOUNDS_PLAY="${CMAKE_CURRENT_SOURCE_DIR}/test-voice-sounds-play.sh")

add_definitions(-DJSON_SIMPLE="${CMAKE_CURRENT_SOURCE_DIR}/test-menu-input-simple.json")
add_definitions(-DJSON_SHORTCUTS="${CMAKE_CURRENT_SOURCE_DIR}/test-menu-input-shortcuts.json")
add_definitions(-DJSON_SOURCE_ONE="${CMAKE_CURRENT_SOURCE_DIR}/test-app-indicator-source-one.json")
add_definitions(-DJSON_SOURCE_TWO="${CMAKE_CURRENT_SOURCE_DIR}/test-app-indicator-source-two.json")
add_definitions(-DJSON_SOURCE_SESSION="${CMAKE_CURRENT_SOURCE_DIR}/test-indicator-source-session.json")
add_definitions(-DJSON_SOURCE_DATETIME="${CMAKE_CURRENT_SOURCE_DIR}/test-indicator-source-datetime.json")
add_definitions(-DJSON_SOURCE_SOUND="${CMAKE_CURRENT_SOURCE_DIR}/test-indicator-source-sound.json")
add_definitions(-DJSON_SOURCE="${CMAKE_CURRENT_SOURCE_DIR}/test-source.json")

add_definitions(-DPRONOUCE_DICT_HASHES="${CMAKE_CURRENT_SOURCE_DIR}/test-pronounce-dict-hashes.dic")
add_definitions(-DPRONOUCE_DICT_SEMICOLON="${CMAKE_CURRENT_SOURCE_DIR}/test-pronounce-dict-semicolon.dic")
add_definitions(-DPRONOUCE_DICT_LOWERCASE="${CMAKE_CURRENT_SOURCE_DIR}/test-pronounce-dict-lowercase.dic")
add_definitions(-DPRONOUCE_DICT_HTK="${CMAKE_CURRENT_SOURCE_DIR}/test-pronounce-dict-htk.dic")

#######################
# Generated test fixtures
#######################

find_package(Gettext)

set(KEYWORD_LOCALE_DIR "${CMAKE_CURRENT_BINARY_DIR}/keyword-mapping/locale")
add_definitions(-DKEYWORD_LOCALE_DIR="${KEYWORD_LOCALE_DIR}")
add_custom_command(
  OUTPUT
    "${KEYWORD_LOCALE_DIR}/en_FAKELANG/LC_MESSAGES"
    "${KEYWORD_LOCALE_DIR}/en_FAKELANG/LC_MESSAGES/gnome-terminal.mo"
  COMMAND
    mkdir -p "${KEYWORD_LOCALE_DIR}/en_FAKELANG/LC_MESSAGES"
  COMMAND
	"${GETTEXT_MSGFMT_EXECUTABLE}"
	--check
	--output-file "${KEYWORD_LOCALE_DIR}/en_FAKELANG/LC_MESSAGES/gnome-terminal.mo"
	"${CMAKE_CURRENT_SOURCE_DIR}/keyword-mapping/gnome-terminal-en_FAKELANG.po"
  VERBATIM
)
add_custom_target(keyword-files DEPENDS "${KEYWORD_LOCALE_DIR}/en_FAKELANG/LC_MESSAGES/gnome-terminal.mo")
	
#######################
# HUD Test Suite
#######################

hudtest(test-watchdog test-watchdog.c test-watchdog.xml)
hudtest(test-distance test-distance.c test-distance.xml)
hudtest(test-hud-item test-huditem.c test-hud-item.xml)
hudtest(test-result-highlighting test-result-highlighting.c test-result-highlighting.xml)
hudtest(test-menu-input test-menu-input.c test-menu-input.xml)
hudtest(test-keyword-mapping test-keyword-mapping.c test-keyword-mapping.xml)
add_dependencies(test-keyword-mapping keyword-files)
hudtest(test-string-list test-string-list.c test-string-list.xml)
hudtest(test-pronounce-dict test-pronounce-dict.c test-pronounce-dict.xml)
# FIXME: Disabled temporarily on platform-api systems due to some platform-api problems
if(NOT(${ENABLE_PLATFORM_API}))
  hudtest(test-source test-source.c test-source.xml)
endif()
hudtest(test-app-indicator-source test-app-indicator-source.c test-app-indicator-source.xml)
hudtest(test-indicator-source test-indicator-source.c test-indicator-source.xml)
if(NOT(${ENABLE_PLATFORM_API}))
  hudtest(test-application-list test-application-list.c test-application-list.xml)
endif()
hudtest(test-application-source test-application-source.c test-application-source.xml)
#test-usage-db-simple.xml
#test-usage-db-old.xml
#test-usage-db-testapp.xml
#test-usage-db-off.xml
#test-bad-app-info-dual-header
#test-bad-app-info-item-no-count
#test-bad-app-info-item-no-name
#test-bad-app-info-menu-no-name
#test-bad-app-info-missing-desktop
#test-bad-app-info-missing-menus
#test-bad-app-info-multiple-menus
#test-good-app-info-tons-of-entries

####################################
# Voice tests (needs working pulse)
####################################

if(${ENABLE_VOICE_TESTS})
  hudtest(test-voice test-voice.c test-voice.xml)
endif()

#######################
# Slow test
#######################

add_executable(hud-performance hud-performance.c)
target_link_libraries(hud-performance
  hud-service
  hud-test
  ${GIO2_LIBRARIES}
)
if(${ENABLE_SCALABILITY_TESTS})
  add_test(hud-performance
    "${CMAKE_CURRENT_SOURCE_DIR}/test-runner.sh"
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}/hud-performance"
    "${CMAKE_CURRENT_BINARY_DIR}/hud-performance.xml"
  )
endif()

#####################################
# libhud-client test suite
#####################################

hudtest(test-hud-client-connection test-hud-client-connection.c test-hud-client-connection.xml)
hudtest(test-hud-client-query test-hud-client-query.c test-hud-client-query.xml)
hudtest(test-hud-client-param test-hud-client-param.c test-hud-client-param.xml)

#####################################
# libhud test suite
#####################################

hudtest(test-hud-manager test-hud-manager.c test-hud-manager.xml)
hudtest(test-hud-action-publisher test-hud-action-publisher.c test-hud-action-publisher.xml)

