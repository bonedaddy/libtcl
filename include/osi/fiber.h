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

#include "osi/conf.h"

/*!@public
 *
 * @brief
 * Represent a fiber unique id.
 */
typedef uint16_t fid_t;

/*!@public
 *
 * @brief
 * Fiber creation attributes.
 */
typedef struct fiber_attr fiber_attr_t;

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
 * Change the behavior of a fiber
 */
enum fiber_flags {

	/*! Act normally */
	FIBER_FL_NONE = 1 << 0,

	/*! Loop through the fiber work without ending context */
	FIBER_FL_LOOP = 1 << 1
};

typedef enum {

	/*! The fiber was just created so ready */
	FIBER_PENDING,

	/*! The fiber is running in it's own context until it finish or yield */
	FIBER_RUNNING,

	/*! TODO */
	FIBER_BLOCKING,

	/*! The fiber is terminated but still exists */
	FIBER_DONE,

	/*! TODO */
	FIBER_DESTROYED
} fiber_st_t;

/*!@public
 *
 * @brief
 * Fiber creation attributes structure definition.
 */
struct fiber_attr {

	/*! The argument which is used on the first call of the fiber. */
	void *context;

	/*! The fiber priority. */
	int priority;

	/*! Can be `FIBER_FL_LOOP' or 0. */
	uint8_t flags;

	/*! Desired stack size, 1024 by default. */
	uint16_t stack_size;
};

/*!@public
 *
 * @brief
 * Creates the new fiber, which will execute the given fn after
 * calling the `fiber_call'.
 * ss (stack size) is the size of the stack for the given fiber.
 * If it is set to 0, then the stack size will be set automatically.
 *
 * @param fid  The fiber to initialize.
 * @param work The fiber function.
 * @param ss   The stack size of the new fiber.
 * @param attr The fiber creation attributes.
 */
__api__ void fiber_init(fid_t *fid, work_t *work, fiber_attr_t attr);

/*!@public
 *
 * @brief
 * Delete the fiber `fiber' and its context.
 *
 * @param fid The fiber to delete.
 */
__api__ void fiber_destroy(fid_t fid);


/*!@public
 *
 * @brief
 * This suspends the current fiber and executes the called one until it reaches
 * the end of its body or until it passes control to yet another fiber.
 * If it reaches the end of its body, it is considered done.
 * `arg' is the callback argument on the first fiber call, then it come the
 * result of `fiber_yield'.
 *
 * @param fid The fiber to resume.
 * @param context   The argument to send to `fiber'.
 * @return      The yielded argument of the final result of the fiber callback.
 */
__api__ void *fiber_call(fid_t fid, void *context);

/*!@public
 *
 * @brief
 * Waits while the given fiber will be finished,
 * i.e. it will exit the `work', which was passed to the `fiber_init'.
 * This function returns immediately in two cases:
 *   1) if the fiber was created but wasn't yet started by calling the
 *     `fiber_call';
 *   2) if the fiber was already finished by leaving the `work'.
 *
 * @param fiber The fiber to join.
 */
__api__ void fiber_join(fid_t fiber);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param fid
 */
__api__ void fiber_unlock(fid_t fid);

/*!@public
 *
 * @brief
 * Tell if `fiber' reaches the end of its body.
 *
 * @param fiber The fiber to check for done.
 * @return      If the fiber is done.
 */
__api__ bool fiber_isdone(fid_t fiber);

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
 * @param context The argument which is the result of `fiber_call'.
 * @return    The argument of `fiber_call' after yielding.
 */
__api__ void *fiber_yield(void *context);

/*!@private
 *
 * @brief
 * TODO
 *
 * @return
 */
__api__ fid_t fiber_getfid(void);

/*!@public
 *
 * @brief
 * TODO
 */
__api__ void fiber_cleanup(void);

__api__ void fiber_schedule(void);

__api__ void fiber_setstate(fiber_st_t st);

__api__ void fiber_setpriority(fid_t fid, int priority);

#endif /* __OSI_FIBER_H */
/*!@} */
