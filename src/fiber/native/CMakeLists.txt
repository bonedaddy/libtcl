check_include_file(ucontext.h HAS_UCONTEXT_H)

if (HAS_UCONTEXT_H)
  add_subdirectory(unix)
elseif (WIN32)
  add_subdirectory(windows)
else ()
  add_subdirectory(hw)
endif ()

set(osi_SRCS ${osi_SRCS}
  ${CMAKE_CURRENT_LIST_DIR}/event.c
  ${CMAKE_CURRENT_LIST_DIR}/event.h
  ${CMAKE_CURRENT_LIST_DIR}/fibers.c
  ${CMAKE_CURRENT_LIST_DIR}/fibers.h
  ${CMAKE_CURRENT_LIST_DIR}/internal.h
  ${CMAKE_CURRENT_LIST_DIR}/sched.h)
set(osi_SRCS ${osi_SRCS} PARENT_SCOPE)
