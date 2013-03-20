add_subdirectory(man)
add_subdirectory(libhud-client)

add_custom_target(doc ALL)
add_dependencies(doc man-pages doc-libhud-client)
