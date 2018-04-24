cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
cmake_policy(SET CMP0054 OLD)
cmake_policy(SET CMP0045 OLD)

set(CMAKE_C_STANDARD 99)

project(osi C)

set(osi_INC_DIR ${CMAKE_CURRENT_LIST_DIR}/include)
set(osi_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
set(osi_TEST_DIR ${CMAKE_CURRENT_LIST_DIR}/test)

include(CheckIncludeFile)
include(CheckFunctionExists)
add_subdirectory(src)

configure_file(${osi_INC_DIR}/osi/conf.h.in
  ${osi_INC_DIR}/osi/conf.h)

file(GLOB_RECURSE osi_HDRS ${osi_HDRS} ${osi_INC_DIR}/osi/*.h)
set(osi_HDR ${osi_INC_DIR}/osi.h)

add_library(osi ${osi_SRCS} ${osi_HDRS} ${osi_HDR})
target_include_directories(osi PUBLIC ${osi_INC_DIR} PRIVATE ${osi_SRC_DIR})
target_link_libraries(osi PUBLIC pp)
add_dependencies(osi pp)

if (MSVC)
  target_compile_options(osi
    PUBLIC /MT$<$<CONFIG:Debug>:d> /Oy /Za /W3
    PUBLIC /D_CRT_SECURE_NO_WARNINGS
    PUBLIC /O$<$<CONFIG:Debug>:d>$<$<CONFIG:Release>:x>)
else ()
  target_compile_options(osi
    PUBLIC -Wall -Werror -Wextra -fomit-frame-pointer -ansi
    PUBLIC -Wno-missing-field-initializers
    PUBLIC -O$<$<CONFIG:Debug>:0 -g3>$<$<CONFIG:Release>:3>)
endif ()

if (ENABLE_TEST)
  add_subdirectory(test)
endif ()

install(FILES ${osi_HDRS} DESTINATION include/osi)
install(FILES ${osi_HDR} DESTINATION include)
