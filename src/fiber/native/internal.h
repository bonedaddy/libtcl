/*
 * Copyright 2018 Tempow
 *
 * Author - 2018 uael <abel@tempow.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*!@file fiber/native/internal.h
 * @author uael
 *
 * @addtogroup osi.fiber.native @{
 */
#ifndef __OSI_FIBER_NATIVE_INTERNAL_H
# define __OSI_FIBER_NATIVE_INTERNAL_H

#include <osi/fiber.h>
#include <osi/conf.h>
#include <osi/stack.h>

#ifdef HAS_UCONTEXT_H
# include <ucontext.h>
#endif

#ifdef OS_WIN
# define _WIN32_WINNT 0x0502
# define WIN32_LEAN_AND_MEAN

# include <windows.h>
# include <winsock2.h>
# include <Mswsock.h>
# include <ws2tcpip.h>
#endif

#include "event.h"

struct osi_fiber
{
	osi_event_t *stop_ev;

#ifdef HAS_UCONTEXT_H
	void *stack;
	ucontext_t context;
#elif defined(OS_WIN)
	LPVOID handle;
#endif
};

OSI_STACK_DECLARE(__api__, osi_fibers, osi_fiber_t, uint16_t)

#endif /* __OSI_FIBER_NATIVE_INTERNAL_H */
/*!@} */
