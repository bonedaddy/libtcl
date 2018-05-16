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

/*!@file osi/fiber/pool.h
 * @author uael
 *
 * @addtogroup osi.fiber @{
 */
#ifndef __OSI_FIBER_POOL_H
# define __OSI_FIBER_POOL_H

#include <osi/fiber.h>

/*!@public
 *
 * @brief
 * The fiber pool structure declaration.
 */
typedef struct fiber_pool fiber_pool_t;

/*!@public
 *
 * @brief
 * The fiber pool structure definition.
 */
struct fiber_pool {

	/*! Memory where fibers are actualy allocated */
	fiber_t *fibers;

	/*! Current length of the `fibers' buffer */
	size_t slot;

	/*! Current capacity of the `fibers' buffer */
	size_t size;

	/*! The ready fiber stack, ordered by priority */
	list_t ready;

	/*! The dead stack of fibers */
	list_t dead;
};

/*!@public
 *
 * @brief
 * TODO
 *
 * @param pool
 */
__api__ void fiber_pool_init(fiber_pool_t *pool);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param pool
 */
__api__ void fiber_pool_destroy(fiber_pool_t *pool);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param pool
 * @return
 */
__api__ fiber_t *fiber_pool_new(fiber_pool_t *pool);

__api__ void fiber_pool_ready(fiber_pool_t *pool, fiber_t *fiber);

__api__ void fiber_pool_dead(fiber_pool_t *pool, fiber_t *fiber);

__api__ fiber_t *fiber_pool_pop(fiber_pool_t *pool);

#endif /* __OSI_FIBER_POOL_H */
/*!@} */
