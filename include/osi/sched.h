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

/*!@file osi/sched.h
 * @author uael
 *
 * @addtogroup osi.sched @{
 */
#ifndef __OSI_SCHED_H
# define __OSI_SCHED_H

#include "osi/fiber/pool.h"

/*!@public
 *
 * @brief
 * The scheduler structure.
 */
typedef struct sched sched_t;

/*!@public
 *
 * @brief
 * The scheduler structure implementation.
 */
struct sched {

	/*! Scheduler fiber pool */
	fiber_pool_t pool;

	/*! If the scheduler is running or not */
	bool scheduled;
};

__api__ sched_t *stdsched;

/*!@public
 *
 * @brief
 * TODO
 *
 * @param sched
 */
__api__ void sched_init(sched_t *sched);

/*!@public
 *
 * @brief
 * TODO
 */
__api__ void sched_start(sched_t *sched);

/*!@public
 *
 * @brief
 * TODO
 */
__api__ void sched_stop(sched_t *sched);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param sched
 * @param fiber
 * @param arg
 * @param prio
 * @return
 */
__api__ fiber_t *sched_spawn(sched_t *sched, work_t *work, uint16_t ss,
	void *arg, int prio);

/*!@public
 *
 * @brief
 * TODO
 *
 * @param sched
 * @param fiber
 * @param arg
 * @param prio
 * @return
 */
__api__ fiber_t *sched_loop(sched_t *sched, work_t *work, uint16_t ss,
	void *arg, int prio);

#endif /* __OSI_SCHED_H */
/*!@} */
