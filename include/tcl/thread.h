/*
 * Copyright (C) 2014 Google, Inc.
 * Copyright (C) 2018 Tempow
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

/*!@file tcl/thread.h
 * @author uael
 *
 * @addtogroup tcl.thread @{
 */
#ifndef __TCL_THREAD_H
# define __TCL_THREAD_H

#include "tcl/reactor.h"
#include "tcl/blocking_queue.h"
#include "tcl/stack.h"
#include "tcl/task.h"

#define THREAD_NAME_MAX 16

/*!@public
 *
 * @brief
 * The thread structure declaration.
 */
typedef struct thread thread_t;

/*!@public
 *
 * @brief
 * The thread structure definition.
 */
struct thread {

	char name[THREAD_NAME_MAX];

	blocking_queue_t work_queue;

	reactor_t reactor;

	task_t task;
};

__api int thread_init(thread_t *thread, char const *name);

__api void thread_destroy(thread_t *thread);

__api int thread_setpriority(thread_t *thread, int priority);

__api void thread_join(thread_t *thread);

__api bool thread_post(thread_t *thread, routine_t *work, void *context);

__api void thread_stop(thread_t *thread);

#endif /* __TCL_THREAD_H */
/*!@} */