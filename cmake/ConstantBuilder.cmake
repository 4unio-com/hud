cmake_minimum_required(VERSION 2.6)
if(POLICY CMP0011)
  cmake_policy(SET CMP0011 NEW)
endif(POLICY CMP0011)

#FIXME This is not a stable macro (i.e. it won't do nothing on un-changed input)
macro(add_constant_template outfiles name const_name input)
  add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${name}.h" "${CMAKE_CURRENT_BINARY_DIR}/${name}.c"
    COMMAND echo "\#include \"${name}.h\"" > "${CMAKE_CURRENT_BINARY_DIR}/${name}.c"
    COMMAND echo -n "const gchar * ${const_name} = \"" >> "${CMAKE_CURRENT_BINARY_DIR}/${name}.c"
    COMMAND cat "${input}" | xargs printf "%s " >> "${CMAKE_CURRENT_BINARY_DIR}/${name}.c"
    COMMAND echo "\";" >> "${CMAKE_CURRENT_BINARY_DIR}/${name}.c"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${ARGN} "${input}"
    VERBATIM
  )
  list(APPEND ${outfiles} "${CMAKE_CURRENT_BINARY_DIR}/${name}.c")
endmacro(add_constant_template)
