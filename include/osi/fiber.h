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

#pragma once

/*!@file osi/fiber.h
 * @author uael
 *
 * @addtogroup osi.fiber @{
 */
#ifndef __OSI_FIBER_H
# define __OSI_FIBER_H

#include <osi/list.h>

#if defined(USE_CORO)
# include <coro.h>
#elif defined(USE_PICORO)
# include <picoro.h>
#elif defined(OS_PROVENCORE)
# include <threads/threads.h>
#else
# error "Unable to implement software coroutines"
#endif

/*!@public
 *
 * @brief
 * The fiber structure.
 */
typedef struct fiber fiber_t;

/*!@private
 *
 * @brief
 * Fiber status.
 */
typedef enum fiber_st fiber_st_t;

/*!@public
 *
 * @brief
 * Declaration of fiber function, which should be passed to the
 * `fiber_new'.
 */
typedef void *(work_t)(void *arg);

/*!@public
 *
 * @brief
 * The fiber status definition
 */
enum fiber_st {

	/*! The fiber was just created so ready */
	OSI_FIB_READY,

	/*! The fiber is running in it's own context until it finish or yield */
	OSI_FIB_RUNNING,

	/*! The fiber is terminated but still exists */
	OSI_FIB_EXITING
};

/*!@public
 *
 * @brief
 * Change the behavior of a fiber
 */
enum fiber_flags {

	/*! Act normally */
	FIBER_NONE = 1 << 0,

	/*! Loop through the fiber work without ending context */
	FIBER_LOOP = 1 << 1
};

/*!@public
 *
 * @brief
 * The fiber structure definition
 */
struct fiber {

	/*! The fiber status */
	fiber_st_t status;

	/*! The fiber core function */
	work_t *fn;

	/*! The fiber core function argument */
	void *arg;

	/*! The fiber result */
	void *result;

	/*! The fiber caller */
	fiber_t *caller;

	/*! Fiber list hold */
	node_t hold;

	/*! The priority used by scheduler */
	int priority;

#if defined(USE_CORO)

	/** Coroutine context */
	coro_context context;

	/** Coroutine stack */
	struct coro_stack stack;
#elif defined(USE_PICORO)

	/** Picoro coroutine context */
	coro context;

	/** Picoro coroutine stack size */
	size_t ss;
#elif defined(OS_PROVENCORE)

	/** ProveNCore coroutine context */
	struct context *context;

	/** ProveNCore coroutine stack size */
	uint16_t ss;
#endif
};

/*!@public
 *
 * @brief
 * Creates the new fiber, which will execute the given fn after
 * calling the `fiber_call'.
 * ss (stack size) is the size of the stack for the given fiber.
 * If it is set to 0, then the stack size will be set automatically.
 *
 * @param fiber The fiber to initialize.
 * @param fn    The fiber function.
 * @param ss    The stack size of the new fiber.
 * @param flags The flags which define this fiber.
 */
__api__ void fiber_init(fiber_t *fiber, work_t *fn, uint16_t ss, uint8_t flags);

/*!@public
 *
 * @brief
 * Delete the fiber `fiber' and its context.
 *
 * @param fiber The fiber to delete.
 */
__api__ void fiber_destroy(fiber_t *fiber);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param fib
 * @param fn
 * @return
 */
__api__ int fiber_reuse(fiber_t *fib, work_t *fn, uint8_t flags);

/*!@public
 *
 * @brief
 * This suspends the current fiber and executes the called one until it reaches
 * the end of its body or until it passes control to yet another fiber.
 * If it reaches the end of its body, it is considered done.
 * `arg' is the callback argument on the first fiber call, then it come the
 * result of `fiber_yield'.
 *
 * @param fiber The fiber to resume.
 * @param arg   The argument to send to `fiber'.
 * @return      The yielded argument of the final result of the fiber callback.
 */
__api__ void *fiber_call(fiber_t *fiber, void *arg);

/*!@public
 *
 * @brief
 * Tell if `fiber' reaches the end of its body.
 *
 * @param fiber The fiber to check for done.
 * @return      If the fiber is done.
 */
__api__ bool fiber_isdone(fiber_t *fiber);

/*!@public
 *
 * @brief
 * The main difference between fibers and functions is that a fiber can be
 * suspended in the middle of its operation and then resumed later. Calling
 * another fiber is one way to suspend a fiber, but that’s more or less the
 * same as one function calling another.
 *
 * Things get interesting when a fiber yields. A yielded fiber passes control
 * back to the fiber that ran it, but remembers where it is. The next time the
 * fiber is called, it picks up right where it left off and keeps going.
 *
 * @param arg The argument which is the result of `fiber_call'.
 * @return    The argument of `fiber_call' after yielding.
 */
__api__ void *fiber_yield(void *arg);

#endif /* __OSI_FIBER_H */
/*!@} */
