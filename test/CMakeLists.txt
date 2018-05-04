macro (osi_register_test name)
  add_executable(test_osi_${name} ${name}.c)
  target_link_libraries(test_osi_${name} osi)
  add_test(test_osi_${name} test_osi_${name})
  add_dependencies(check test_osi_${name})
  add_dependencies(test_osi_${name} osi)
endmacro ()  

osi_register_test(fiber_adjectives)
osi_register_test(fiber_call)
osi_register_test(fiber_call_return_value)
osi_register_test(fiber_call_to_parameter)
osi_register_test(fiber_is_done)
osi_register_test(fiber_resume_caller)
osi_register_test(fiber_yield_return_value)

osi_register_test(sched_yield_no_arg)

osi_register_test(event_post)
