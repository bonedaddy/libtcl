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

/*!@file osi/fiber.h
 * @author uael
 *
 * @addtogroup osi.fiber @{
 */
#ifndef __OSI_FIBER_H
# define __OSI_FIBER_H

#include <osi/conf.h>
#include <pp.h>

/*!@public
 *
 * @brief
 * The fiber opaque structure.
 */
typedef struct fiber fiber_t;

/*!@public
 *
 * @brief
 * Declaration of fiber function, which should be passed to the
 * `fiber_new'.
 */
typedef void *(fiber_fn_t)(void *arg);

/*!@public
 *
 * @brief
 * Creates the new fiber, which will execute the given fn after
 * calling the `sched_start'.
 * ss (stack size) is the size of the stack for the given fiber.
 * If it is set to 0, then the stack size will be set automatically.
 *
 * @param fn The fiber function.
 * @param ss The stack size of the new fiber.
 * @return   The new fiber.
 */
__api__ fiber_t *fiber_new(fiber_fn_t *fib, uint16_t ss);

/*!@public
 *
 * @brief
 * Delete the fiber `fib' and its context.
 *
 * @param fib The fiber to delete.
 */
__api__ void fiber_del(fiber_t *fib);

/*!@public
 *
 * @brief
 * This suspends the current fiber and executes the called one until it reaches
 * the end of its body or until it passes control to yet another fiber.
 * If it reaches the end of its body, it is considered done.
 * `arg' is the callback argument on the first fiber call, then it come the
 * result of `fiber_yiled'.
 *
 * @param fib The fiber to resume.
 * @param arg The argument to send to `fib'.
 * @return    The yielded argument of the final result of the fiber callback.
 */
__api__ void *fiber_call(fiber_t *fib, void *arg);

/*!@public
 *
 * @brief
 * Tell if `fib' reaches the end of its body.
 *
 * @param fib The fiber to check for done.
 * @return    If the fiber is done.
 */
__api__ bool fiber_isdone(fiber_t *fib);

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
