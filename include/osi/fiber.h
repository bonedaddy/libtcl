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
 * `osi_fibctor'.
 */
typedef void (osi_fibfn_t)(void *arg);

/*!@public
 *
 * @brief
 * Creates the new fiber, which will execute the given fn after
 * calling the `osi_schedule'.
 * ss (stack size) is the size of the stack for the given fiber.
 * If it is set to 0, then the stack size will be set automatically.
 *
 * @param fn   The fiber function.
 * @param arg  The fiber function argument.
 * @param ss   The stack size of the new fiber.
 * @param prio The priority of the new fiber.
 * @return     The new fiber.
 */
__api__ osi_fib_t *osi_fib_create(osi_fibfn_t *, void *, uint16_t, uint8_t);

#endif /* __OSI_FIBER_H */
/*!@} */
