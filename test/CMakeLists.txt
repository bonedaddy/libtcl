macro (osi_register_test name)
  add_executable(test_osi_${name} ${name}.c)
  target_link_libraries(test_osi_${name} osi)
  add_test(test_osi_${name} test_osi_${name})
  add_dependencies(check test_osi_${name})
  add_dependencies(test_osi_${name} osi)
endmacro ()  

osi_register_test(fiber)
