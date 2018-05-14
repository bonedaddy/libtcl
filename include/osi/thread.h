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

/*!@file osi/thread.h
 * @author uael
 *
 * @addtogroup osi.thread @{
 */
#ifndef __OSI_THREAD_H
# define __OSI_THREAD_H

#include <osi/reactor.h>
#include <osi/event.h>

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

	char name[THREAD_NAME_MAX + 1];

#ifdef OSI_THREAD_MOD

	bool is_joined;

	pthread_t pthread;

	pid_t tid;

	reactor_t reactor;

	equeue_t work_queue;
#else

	list_t fibers;
#endif
};

int thread_init(thread_t *thread, char const *name);

void thread_destroy(thread_t *thread);

void thread_join(thread_t *thread);

bool thread_post(thread_t *thread, work_t *work, void *context);

void thread_stop(thread_t *thread);

bool thread_priority(thread_t *thread, int priority);

#endif /* __OSI_THREAD_H */
/*!@} */
