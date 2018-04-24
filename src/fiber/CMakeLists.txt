if (OS_PROVENCORE)
  message(STATUS "Using provencore fibers")
  add_subdirectory(provencore)
else ()
  message(STATUS "Using native fibers")
  add_subdirectory(native)
endif ()

set(osi_SRCS ${osi_SRCS} PARENT_SCOPE)
