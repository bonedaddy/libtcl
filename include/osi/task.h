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

/*!@file osi/task.h
 * @author uael
 *
 * @addtogroup osi.task @{
 */
#ifndef __OSI_TASK_H
# define __OSI_TASK_H

#include "osi/fiber.h"

/*!@public
 *
 * @brief
 * The task structure declaration.
 */
typedef struct task task_t;

/*!@public
 *
 * @brief
 * The task structure definition.
 */
struct task {
	
	bool running;

	bool joined;

	work_t *work;

	void *context;

#ifdef OSI_THREADING

	pthread_t pthread;
#else

	fid_t fiber;
#endif
};

__api__ int task_spawn(task_t *task, work_t *work, void *context);

__api__ int task_repeat(task_t *task, work_t *work, void *context);

__api__ void task_destroy(task_t *task);

__api__ bool task_running(task_t *task);

__api__ int task_setpriority(task_t *task, int priority);

__api__ void task_stop(task_t *task);

__api__ void task_join(task_t *task);

__api__ void task_schedule(void);

#endif /* __OSI_TASK_H */
/*!@} */
