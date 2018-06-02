set(CORO_X86_64 OFF)
set(CORO_X86_32 OFF)
set(CORO_ARM OFF)
set(CORO_UCONTEXT OFF)
set(CORO_WIN OFF)
set(CORO_SJLJ OFF)

if (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
  set(CORO_X86_64 ON)
  set(osi_SRCS ${osi_SRCS}
    ${osi_SRC_DIR}/coro/coro.c
    ${osi_SRC_DIR}/coro/pool.c
    ${osi_SRC_DIR}/coro/coro-x86_64.c)

  message(STATUS "Selecting `x86_64' coroutine implementation.")
elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "x86")
  set(CORO_X86_32 ON)
  set(osi_SRCS ${osi_SRCS}
    ${osi_SRC_DIR}/coro/coro.c
    ${osi_SRC_DIR}/coro/pool.c
    ${osi_SRC_DIR}/coro/coro-x86_32.c)

  message(STATUS "Selecting `x86_32' coroutine implementation.")
elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "arm")
  set(CORO_ARM ON)
  set(osi_SRCS ${osi_SRCS}
    ${osi_SRC_DIR}/coro/coro.c
    ${osi_SRC_DIR}/coro/pool.c
    ${osi_SRC_DIR}/coro/coro-arm.c)

  message(STATUS "Selecting `arm' coroutine implementation.")
elseif (HAS_UCONTEXT_H)
  set(CORO_UCONTEXT ON)
  set(osi_SRCS ${osi_SRCS}
    ${osi_SRC_DIR}/coro/coro.c
    ${osi_SRC_DIR}/coro/pool.c
    ${osi_SRC_DIR}/coro/coro-ucontext.c)

  message(STATUS "Selecting `ucontext' coroutine implementation.")
elseif (MSVC)
  set(CORO_WIN ON)
  set(osi_SRCS ${osi_SRCS} ${osi_SRC_DIR}/coro/coro-win.c)

  message(STATUS "Selecting `windows' coroutine implementation.")
elseif (HAS_SETJMP_H)
  set(CORO_SJLJ ON)
  set(osi_SRCS ${osi_SRCS} ${osi_SRC_DIR}/coro/coro-sjlj.c)

  message(STATUS "Selecting `setjmp/longjmp' coroutine implementation.")
else ()
  message(FATAL_ERROR "Unable to implement coroutine in your system.")
endif ()

set(CORO_X86_64 ${CORO_X86_64} PARENT_SCOPE)
set(CORO_X86_32 ${CORO_X86_32} PARENT_SCOPE)
set(CORO_ARM ${CORO_ARM} PARENT_SCOPE)
set(CORO_UCONTEXT ${CORO_UCONTEXT} PARENT_SCOPE)
set(CORO_WIN ${CORO_WIN} PARENT_SCOPE)
set(CORO_SJLJ ${CORO_SJLJ} PARENT_SCOPE)

set(osi_SRCS ${osi_SRCS} PARENT_SCOPE)
