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
#include <osi/equeue.h>

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

	bool is_joined;

	equeue_t work_queue;

#ifdef OSI_THREADING

	pthread_t pthread;

	reactor_t reactor;
#else

	fiber_t fiber;
#endif
};

__api__ int thread_init(thread_t *thread, char const *name);

__api__ void thread_destroy(thread_t *thread);

__api__ void thread_join(thread_t *thread);

__api__ bool thread_post(thread_t *thread, work_t *work, void *context);

__api__ void thread_stop(thread_t *thread);

//TODO TEMPOW
typedef struct reactor_t reactor_t;
reactor_t *thread_get_reactor(const thread_t *thread);
bool thread_set_priority(thread_t *thread, int priority);
thread_t *thread_new(const char *name);
void thread_free(thread_t *thread);
typedef void (*thread_fn)(void *context);
bool thread_post_old(thread_t *thread, thread_fn func, void *context);

#endif /* __OSI_THREAD_H */
/*!@} */
