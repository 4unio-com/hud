include(UseMscgen)

add_subdirectory(man)
add_subdirectory(libhud-client)

install(
  FILES
    "HUD Architecture.svg"
  DESTINATION
    ${CMAKE_INSTALL_DOCDIR}
)

mscgen(libhud-communication INSTALL)
mscgen(libhud-client INSTALL)
