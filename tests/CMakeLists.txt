macro(hudtest tname tfilename)
  add_executable(${tname} ${tfilename})
  target_link_libraries(${tname} hud-service)
  add_test(${tname} ${tname})
endmacro()

hudtest(string-list test-string-list.c)

#add_executable(lev_scalability LevScalabilityTest.cc)
#target_link_libraries(lev_scalability ${COL_LIB_BASENAME})
#if(${enable_scalability_tests})
#  add_test(lev_scalability lev_scalability /usr/share/dict/words)
#endif()

#add_executable(create_performance CreatePerformanceTest.cc)
#target_link_libraries(create_performance ${COL_LIB_BASENAME})
