# **************************************************************************** #
#                                                                              #
# Copyright 2018 Tempow                                                        #
#                                                                              #
# Author - 2018 uael <abel@tempow.com>                                         #
#                                                                              #
# Licensed under the Apache License, Version 2.0 (the "License");              #
# you may not use this file except in compliance with the License.             #
# You may obtain a copy of the License at                                      #
#                                                                              #
#     http://www.apache.org/licenses/LICENSE-2.0                               #
#                                                                              #
# Unless required by applicable law or agreed to in writing, software          #
# distributed under the License is distributed on an "AS IS" BASIS,            #
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.     #
# See the License for the specific language governing permissions and          #
# limitations under the License.                                               #
#                                                                              #
# **************************************************************************** #

cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
cmake_policy(SET CMP0054 OLD)
cmake_policy(SET CMP0045 OLD)

set(CMAKE_C_STANDARD 99)

project(osi C)

set(osi_INC_DIR ${CMAKE_CURRENT_LIST_DIR}/include)
set(osi_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
set(osi_TEST_DIR ${CMAKE_CURRENT_LIST_DIR}/test)

option(OSI_THREADING "Enable Logging" ON)
if (CMAKE_BUILD_TYPE MATCHES Release)
  option(OSI_LOGGING "Enable Logging" OFF)
else ()
  option(OSI_LOGGING "Enable Logging" OFF)
endif ()

include(CheckIncludeFile)
check_include_file(sys/eventfd.h HAS_SYS_EVENTFD_H)
check_include_file(sys/epoll.h HAS_SYS_EPOLL_H)
check_include_file(sys/time.h HAS_SYS_TIME_H)
check_include_file(sys/prctl.h HAS_SYS_PRCTL_H)
check_include_file(sys/_system_properties.h HAS_SYS__SYSTEM_PROPERTIES_H)
check_include_file(pthread.h HAS_PTHREAD_H)
check_include_file(unistd.h HAS_UNISTD_H)
check_include_file(time.h HAS_TIME_H)
check_include_file(fcntl.h HAS_FCNTL_H)
check_include_file(signal.h HAS_SIGNAL_H)
check_include_file(stdbool.h HAS_STDBOOL_H)
check_include_file(stdint.h HAS_STDINT_H)
check_include_file(strings.h HAS_STRINGS_H)

include(CheckFunctionExists)
check_function_exists(bzero HAS_BZERO)
check_function_exists(free HAS_FREE)
check_function_exists(realloc HAS_REALLOC)
check_function_exists(strnlen HAS_STRNLEN)
check_function_exists(strlcpy HAS_STRLCPY)
check_function_exists(strncpy HAS_STRNCPY)

check_function_exists(wakelock_acquire HAS_WAKELOCK_ACQUIRE)
check_function_exists(wakelock_release HAS_WAKELOCK_RELEASE)

if (OSI_THREADING AND (NOT HAS_SYS_EVENTFD_H OR NOT HAS_SYS_EPOLL_H
  OR NOT HAS_PTHREAD_H OR NOT HAS_UNISTD_H OR NOT HAS_SYS_PRCTL_H
  OR NOT HAS_FCNTL_H))
  set(OSI_THREADING OFF)
endif ()

configure_file(${osi_INC_DIR}/osi/conf.h.in
  ${CMAKE_BINARY_DIR}/include/osi/conf.h @ONLY)

file(GLOB_RECURSE osi_SRCS ${osi_SRCS} ${osi_SRC_DIR}/*.c)
file(GLOB_RECURSE osi_SRCS ${osi_SRCS} ${osi_SRC_DIR}/*.h)
file(GLOB_RECURSE osi_HDRS ${osi_HDRS} ${osi_INC_DIR}/osi/*.h)
set(osi_HDR ${osi_INC_DIR}/osi.h ${CMAKE_BINARY_DIR}/include/osi/conf.h)

add_library(osi ${osi_SRCS} ${osi_HDRS} ${osi_HDR})
target_include_directories(osi
  PUBLIC ${osi_INC_DIR} ${CMAKE_BINARY_DIR}/include
  PRIVATE ${osi_SRC_DIR})
target_link_libraries(osi PUBLIC rt)

if (USE_CORO)
  target_link_libraries(osi PUBLIC coro)
  add_dependencies(osi coro)
elseif (USE_PICORO)
  target_link_libraries(osi PUBLIC picoro)
  add_dependencies(osi picoro)
endif ()

if (OSI_THREADING)
  set(THREADS_PREFER_PTHREAD_FLAG ON)
  find_package(Threads)
  if (Threads_FOUND)
    target_link_libraries(osi PUBLIC Threads::Threads)
  endif ()
endif ()

target_compile_definitions(osi
  INTERFACE OSI_COMPILE)
if (MSVC)
  target_compile_options(osi
    INTERFACE /MT$<$<CONFIG:Debug>:d> /Oy /Za /W3
    INTERFACE /D_CRT_SECURE_NO_WARNINGS
    INTERFACE /O$<$<CONFIG:Debug>:d>$<$<CONFIG:Release>:x>)
  if (CMAKE_BUILD_TYPE MATCHES Release)
    target_compile_options(osi INTERFACE /Ox)
  else ()
    target_compile_options(osi INTERFACE /Od)
  endif ()
else ()
  target_compile_options(osi
    INTERFACE -Wall -Werror -Wextra -fomit-frame-pointer
    INTERFACE -Wno-implicit-function-declaration
    INTERFACE -Wno-missing-field-initializers
    INTERFACE -Wno-pointer-to-int-cast
    INTERFACE -Wno-implicit-fallthrough)
  if (CMAKE_BUILD_TYPE MATCHES Release)
    target_compile_options(osi PRIVATE -O3)
  else ()
    target_compile_options(osi INTERFACE -O0 -g3)
    if (CMAKE_BUILD_TYPE MATCHES San)
      target_link_libraries(osi INTERFACE "-fsanitize=address")
      target_compile_options(osi INTERFACE "-fsanitize=address")
    endif ()
  endif ()
endif ()

if (ENABLE_TEST)
  add_subdirectory(${osi_TEST_DIR})
endif ()

install(FILES ${osi_HDRS} DESTINATION include/osi)
install(FILES ${osi_HDR} DESTINATION include)
