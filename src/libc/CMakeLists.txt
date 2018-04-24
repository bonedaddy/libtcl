macro (add_src_if_not_exists CFUNC)
  check_function_exists(${CFUNC} HAS_${CFUNC})
  if (NOT HAS_${CFUNC})
    if (ARGN)
      set(osi_SRCS ${osi_SRCS} ${ARGN})
    else ()
      set(osi_SRCS ${osi_SRCS} ${CFUNC}.c)
    endif ()
  endif ()
endmacro ()

add_src_if_not_exists(bzero)

set(osi_SRCS ${osi_SRCS} PARENT_SCOPE)
