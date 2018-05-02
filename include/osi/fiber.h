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
typedef struct osi_fib osi_fib_t;

/*!@public
 *
 * @brief
 * Declaration of fiber function, which should be passed to the
 * `osi_fib_new'.
 */
typedef void *(osi_fibfn_t)(void *arg);

/*!@public
 *
 * @brief
 * Creates the new fiber, which will execute the given fn after
 * calling the `osi_schedule'.
 * ss (stack size) is the size of the stack for the given fiber.
 * If it is set to 0, then the stack size will be set automatically.
 *
 * @param fn The fiber function.
 * @param ss The stack size of the new fiber.
 * @return   The new fiber.
 */
__api__ osi_fib_t *osi_fib_new(osi_fibfn_t *fib, uint16_t ss);

/*!@public
 *
 * @brief
 * This suspends the current fiber and executes the called one until it reaches
 * the end of its body or until it passes control to yet another fiber.
 * If it reaches the end of its body, it is considered done.
 * `arg' is the callback argument on the first fiber call, then it come the
 * result of `osi_fib_yield'.
 *
 * @param fib The fiber to resume.
 * @param arg The argument to send to `fib'.
 * @return    The yielded argument of the final result of the fiber callback.
 */
__api__ void *osi_fib_call(osi_fib_t *fib, void *arg);

/*!@public
 *
 * @brief
 * Tell if `fib' reaches the end of its body.
 *
 * @param fib The fiber to check for done.
 * @return    If the fiber is done.
 */
__api__ bool osi_fib_done(osi_fib_t *fib);

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
 * @param arg The argument which is the result of `osi_fib_call'.
 * @return    The argument of `osi_fib_call' after yielding.
 */
__api__ void *osi_fib_yield(void *arg);

#endif /* __OSI_FIBER_H */
/*!@} */
