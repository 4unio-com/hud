add_executable(queryindex queryindex.cc)
target_link_libraries(queryindex ${COL_LIB_BASENAME})

if(GTK3_FOUND)
  include_directories(${GTK3_INCLUDE_DIRS})
  add_executable(singleword singleword.cc)
  target_link_libraries(singleword ${COL_LIB_BASENAME} ${GTK3_LIBRARIES})
  
  add_executable(queryapp queryapp.cc)
  target_link_libraries(queryapp ${COL_LIB_BASENAME} ${GTK3_LIBRARIES})

  add_executable(hudtest hudtest.cc)
  target_link_libraries(hudtest ${COL_LIB_BASENAME} ${GTK3_LIBRARIES})

  add_executable(sctest sctest.cc)
  target_link_libraries(sctest ${COL_LIB_BASENAME} ${GTK3_LIBRARIES})

  add_executable(numberpad numberpad.cc)
  target_link_libraries(numberpad ${COL_LIB_BASENAME} ${GTK3_LIBRARIES})

endif()
