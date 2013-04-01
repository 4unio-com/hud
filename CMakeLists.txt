project(hud C)
cmake_minimum_required(VERSION 2.8.9)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" "${CMAKE_MODULE_PATH}")

set(PACKAGE ${CMAKE_PROJECT_NAME})
set(GETTEXT_PACKAGE ${CMAKE_PROJECT_NAME})

if(PROJECT_BINARY_DIR STREQUAL PROJECT_SOURCE_DIR)
   message(FATAL_ERROR "In-tree build attempt detected, aborting. Set your build dir outside your source dir, delete CMakeCache.txt from source root and try again.")
endif()

option(enable_tests "Enable tests." ON)
option(enable_voice_tests "Enable voice tests" ON)
option(enable_scalability_tests "Additional scalability tests that are potentially very slow to run." OFF)
option(full_warnings "Full compiler warnings." OFF)
option(debug_messages "Print debug messages." OFF)
option(local_install "Support local installation." OFF)

find_package(PkgConfig REQUIRED)
include(GNUInstallDirs)
include(CheckIncludeFile)
include(CheckFunctionExists)
include(Coverage)
include(UseGlibGeneration)
include(UseGdbusCodegen)
include(UseConstantBuilder)
include(UseGSettings)
include(UseVala)

# Workaround for libexecdir on debian
if (EXISTS "/etc/debian_version") 
  set(CMAKE_INSTALL_LIBEXECDIR ${CMAKE_INSTALL_LIBDIR})
  set(CMAKE_INSTALL_FULL_LIBEXECDIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBEXECDIR}")
endif()

if(${full_warnings})
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic")
endif()

if(${debug_messages})
  add_definitions(-DDEBUG_MESSAGES)
endif()

set(HUD_QUERY_XML ${CMAKE_CURRENT_SOURCE_DIR}/data/com.canonical.hud.query.xml)
set(HUD_SERVICE_XML ${CMAKE_CURRENT_SOURCE_DIR}/data/com.canonical.hud.xml)
set(HUD_APP_XML ${CMAKE_CURRENT_SOURCE_DIR}/data/com.canonical.hud.Application.xml)

set(HUD_TOP_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(HUD_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

add_definitions( -DDATADIR="${CMAKE_INSTALL_FULL_DATADIR}" )
add_definitions( -DGNOMELOCALEDIR="${CMAKE_INSTALL_FULL_DATADIR}/locale" )
add_definitions( -DGETTEXT_PACKAGE="${GETTEXT_PACKAGE}" )
add_definitions( -DLIBEXECDIR="${CMAKE_INSTALL_FULL_LIBEXECDIR}" )
add_definitions( -DDICT_PATH="/usr/share/pocketsphinx/model/lm/en_US/cmu07a.dic" )
add_definitions( -DJULIUS_DICT_PATH="/usr/share/julius-voxforge/acoustic/" )

pkg_check_modules(GLIB2 REQUIRED glib-2.0)
include_directories(${GLIB2_INCLUDE_DIRS})

pkg_check_modules(GOBJECT2 REQUIRED gobject-2.0)
include_directories(${GOBJECT2_INCLUDE_DIRS})

pkg_check_modules(GIO2 REQUIRED gio-2.0)
include_directories(${GIO2_INCLUDE_DIRS})

pkg_check_modules(GIO_UNIX2 REQUIRED gio-unix-2.0)
include_directories(${GIO_UNIX2_INCLUDE_DIRS})

pkg_check_modules(DEE REQUIRED dee-1.0)
include_directories(${DEE_INCLUDE_DIRS})

pkg_check_modules(SQLITE3 REQUIRED sqlite3)
include_directories(${SQLITE3_INCLUDE_DIRS})

pkg_check_modules(DBUSMENU_GLIB REQUIRED dbusmenu-glib-0.4)
include_directories(${DBUSMENU_GLIB_INCLUDE_DIRS})

pkg_check_modules(POCKETSPHINX REQUIRED pocketsphinx)
include_directories(${POCKETSPHINX_INCLUDE_DIRS})

pkg_search_module(GTK3 REQUIRED gtk+-3.0)
include_directories(${GTK3_INCLUDE_DIRS})

find_package(Vala 0.12)

find_package(GObjectIntrospection 0.9.12)

check_include_file("signal.h" HAVE_SIGNAL_H)
if(${HAVE_SIGNAL_H})
  set(CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} "signal.h")
  check_function_exists(kill HAVE_KILL)
endif()

find_package(Curses)
check_include_file("readline/readline.h" READLINE_FOUND)
check_include_file("readline/history.h" READLINE_HISTORY_FOUND)
find_library(READLINE_LIBRARIES NAMES readline)

pkg_search_module(PANGO REQUIRED pango)

pkg_search_module(BAMF libbamf3)
if(${BAMF_FOUND})
  add_definitions( -DHAVE_BAMF=1 )
  include_directories(${BAMF_INCLUDE_DIRS})
endif()

if(${local_install})
  set(DBUSSERVICEDIR "${CMAKE_INSTALL_DATADIR}/dbus-1/services/")
else()
  EXEC_PROGRAM(${PKG_CONFIG_EXECUTABLE} ARGS dbus-1 --variable session_bus_services_dir OUTPUT_VARIABLE DBUSSERVICEDIR )
endif()
message("Installing DBus services to ${DBUSSERVICEDIR}")

if(${local_install})
  set(DBUSIFACEDIR "${CMAKE_INSTALL_DATADIR}/dbus-1/interfaces/")
else()
  EXEC_PROGRAM(${PKG_CONFIG_EXECUTABLE} ARGS dbus-1 --variable interfaces_dir OUTPUT_VARIABLE DBUSIFACEDIR )
endif()
message("Installing DBus interfaces to ${DBUSIFACEDIR}")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -fPIC")

add_subdirectory(data)
add_subdirectory(libhud-client)
add_subdirectory(libhud)
add_subdirectory(libhud-gtk)
add_subdirectory(src)
add_subdirectory(tools)

if(${enable_tests})
  enable_testing()
  
  pkg_check_modules(DBUSTEST REQUIRED dbustest-1)
  include_directories(${DBUSTEST_INCLUDE_DIRS})
  
  pkg_check_modules(DBUSMENU_JSONLOADER REQUIRED dbusmenu-jsonloader-0.4)
  include_directories(${DBUSMENU_JSONLOADER_INCLUDE_DIRS})
  
  add_subdirectory(tests)
endif()

add_subdirectory(tools-vala)
add_subdirectory(docs)
