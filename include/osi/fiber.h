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

#include <pp.h>

/*!@public
 *
 * @brief
 * The opaque fiber structure.
 */
typedef struct osi_fiber osi_fiber_t;

/*!@public
 *
 * @brief
 * Declaration of fiber function, which should be passed to the
 * `osi_fiber_ctor'.
 */
typedef void (osi_fiber_fn_t)(osi_fiber_t *fiber, void *arg);

/*!@public
 *
 * @brief
 * Creates the new fiber, which will execute the given fiber_func after
 * calling the `osi_fiber_start'.
 * ss (stack size) is the size of the stack for the given fiber.
 * If it is set to 0, then the stack size will be set automatically.
 *
 * @param fiber The fiber to construct.
 * @param fn    The fiber function.
 * @param ss    The stack size of the new fiber.
 * @return      0 on success 1 otherwise
 */
__api__ int osi_fiber_ctor(osi_fiber_t *fiber, osi_fiber_fn_t *fn, uint16_t ss);

/*!@public
 *
 * @brief
 * Deletes the given fiber.
 *
 * @param fiber The fiber to deconstruct.
 * @return      0 on success 1 otherwise
 */
__api__ int osi_fiber_dtor(osi_fiber_t *fiber);

/*!@public
 *
 * @brief
 * Schedules the given fiber for execution and passes the ctx to the fiber func,
 * which has been set in the `osi_fiber_ctor'.
 *
 * @param fiber The fiber to start.
 * @return      0 on success 1 otherwise
 */
__api__ int	osi_fiber_start(osi_fiber_t *fiber, void *arg);

/*!@public
 *
 * @param fiber
 * @return
 */
__api__ int	osi_fiber_join(osi_fiber_t *fiber);

/*!@public
 *
 * @param fiber
 * @return
 */
__api__ int osi_yield(void);

#endif /* __OSI_FIBER_H */
/*!@} */
