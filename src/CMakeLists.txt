add_subdirectory(libc)
if (NOT HAS_FIBER)
  message(STATUS "Selecting osi fiber")
  add_subdirectory(fiber)
endif ()

set(osi_SRCS ${osi_SRCS} PARENT_SCOPE)
